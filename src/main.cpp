#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <format>
#include <ctime>
#include <utility>
#include <optional>
#include <sstream>

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
    [[nodiscard]] std::optional<Note> find_by_id(long id) const
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

    // 删除成功返回 true；找不到对应 ID 时返回 false。
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
            if (!read_id(input, id))
            {
                std::cout << "Usage: " << command << " <id>\n";
                continue;
            }

            if (command == "show")
            {
                if (const auto note = store.find_by_id(id)) display(*note);
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
        else if (!command.empty())
        {
            std::cout << "Unknown command. Type 'help' for commands.\n";
        }
    }

    return 0;
}
