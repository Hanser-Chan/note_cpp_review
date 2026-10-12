#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <format>
#include <ctime>
#include <utility>
#include <optional>
#include <sstream>
#include <string_view>

enum class NoteStatus
{
    active, // 默认状态 激活
    archived, // 归档
};

struct Note
{
    long id;
    std::string title;
    std::string content;
    time_t create_time;
    time_t update_time;
    NoteStatus status{NoteStatus::active};
};

// TODO stage2/day2：为 Note 选择一种有明确语义的比较或格式化操作。
// 语法参考（二选一即可）：
//   bool operator==(const Note&) const = default;
//   std::ostream& operator<<(std::ostream& out, const Note& note);

class NoteStore {
public:
    long add(std::string title, std::string content) {
        const time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        const long id = next_id_++;
        notes_.push_back(Note{
            .id = id,
            .title = std::move(title),
            .content = std::move(content),
            .create_time = now,
            .update_time = now});
        return id;
    }

    // 只读借用内部集合，避免复制整个 vector。
    [[nodiscard]] const std::vector<Note>& all() const {
        return notes_;
    }

    // 找到时返回 Note 副本，找不到时返回 std::nullopt。
    // TODO stage2/day1：从 main 中分别用整数 ID 和字符串标题调用两个 find 重载，
    // 并确认重载由参数类型决定（不能只用返回类型区分重载）。
    [[nodiscard]] std::optional<Note> find(long id) const
    {
        for (const Note& note : notes_)
        {
            if (note.id == id)
            {
                return note;
            }
        }
        return std::nullopt;
    }

    // TODO stage2/day1：把标题查询参数改为 std::string_view，练习非拥有只读视图。
    // 参考签名：std::optional<Note> find(std::string_view title) const;
    // string_view 不拥有字符；不要把它保存到 NoteStore 或 Note 中。
    [[nodiscard]] std::optional<Note> find(const std::string_view title) const
    {
        for (const Note& note : notes_)
        {
            if (note.title == title)
            {
                return note;
            }
        }
        return std::nullopt;
    }

    // 删除成功返回 true；找不到对应 ID 时返回 false。
    // TODO stage2/day2：说明为什么“未找到”使用 optional/bool 表达，
    // 而不是抛异常；try/catch 留给真正异常的失败情形。std::expected 属于 C++23。
    bool delete_by_id(long id)
    {
        for (auto it = notes_.begin(); it != notes_.end(); ++it)
        {
            if (it->id == id)
            {
                notes_.erase(it);
                return true;
            }
        }
        return false;
    }

private:
    long next_id_{1};
    std::vector<Note> notes_;
};

// TODO stage2/day1（可选）：写一个接收连续只读 Note 序列的辅助函数，练习 std::span。
// 参考签名：void display_notes(std::span<const Note> notes);
// 需要 <span>；span 不拥有元素，不能比底层 vector 活得更久。

static void display(const Note& note)
{
    switch (note.status)
    {
        default:
        case NoteStatus::active:
            std::cout << std::format("{}: {}: {}", note.id, note.title, note.content) << std::endl;
            break;
        case NoteStatus::archived:
            std::cout << std::format("Archived note: {}: {}, archived time: {}", note.id, note.title, note.update_time) << std::endl;
            break;
    }
}

static void print_help()
{
    std::cout << "Commands:\n"
              << "  add       Add a note (title and content are entered on separate lines)\n"
              << "  list      List all notes\n"
              << "  show <id> Show one note\n"
              << "  delete <id> Delete one note\n"
              << "  help      Show this help\n"
              << "  quit      Exit\n";
}

static bool read_id(std::istringstream& input, long& id)
{
    if (!(input >> id)) return false;
    input >> std::ws;
    return input.eof();
}

static bool read_string(std::istringstream& input, std::string& value)
{
    return static_cast<bool>(std::getline(input >> std::ws, value));
}

int main()
{
    NoteStore store;
    std::string line;

    print_help();
    while (std::cout << "> " && std::getline(std::cin, line))
    {
        std::istringstream input(line);
        std::string command;
        input >> command;

        if (command == "quit")
        {
            break;
        }
        if (command == "help")
        {
            print_help();
        }
        else if (command == "add")
        {
            std::string title;
            std::string content;
            std::cout << "Title: ";
            if (!std::getline(std::cin, title)) break;
            std::cout << "Content: ";
            if (!std::getline(std::cin, content)) break;

            // TODO stage2/day2：定义并实现空标题/空正文的校验策略；失败时不新增笔记。
            const long id = store.add(std::move(title), std::move(content));
            std::cout << "Added note " << id << ".\n";
        }
        else if (command == "list")
        {
            if (store.all().empty())
            {
                std::cout << "No notes yet.\n";
            }
            else
            {
                for (const Note& note : store.all()) display(note);
            }
        }
        else if (command == "show" || command == "delete")
        {
            long id{};
            // TODO stage2/day2：手动检查缺少 ID、非整数、溢出和多余字符等输入。
            if (!read_id(input, id))
            {
                std::cout << "Usage: " << command << " <id>\n";
                continue;
            }

            if (command == "show")
            {
                if (const auto note = store.find(id)) display(*note);
                else std::cout << "No note with ID " << id << ".\n";
            }
            else if (store.delete_by_id(id))
            {
                std::cout << "Deleted note " << id << ".\n";
            }
            else
            {
                std::cout << "No note with ID " << id << ".\n";
            }
        }
        else if (command == "find")
        {
            std::string title;
            if (!read_string(input, title))
            {
                std::cout << "Usage: " << command << " <string>\n";
                continue;
            }
            if (const auto note = store.find(title)) display(*note);
        }
        else if (!command.empty())
        {
            // TODO stage2/day2：检查未知命令后，程序能否继续处理下一行输入。
            std::cout << "Unknown command. Type 'help' for commands.\n";
        }
    }

    return 0;
}
