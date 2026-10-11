#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <format>
#include <ctime>
#include <utility>
#include <optional>

// 阶段 1：数据建模与值语义
//
// 建议按 plan/stage1.md 的顺序亲手补充代码：
//   1. 定义 NoteStatus 和 Note
//   2. 实现 NoteStore::add / all
//   3. 实现查找和删除
//   4. 最后再尝试添加简单的命令行交互
//
// 本文件故意只提供练习框架；请把 TODO 替换为自己的实现。

// 1. 定义笔记状态。
// 示例：enum class NoteStatus { active, archived };
enum class NoteStatus
{
    active, // 默认状态 激活
    archived, // 归档
};

// 2. 定义 Note 数据结构。
// 建议字段：int id、std::string title、std::string content、NoteStatus status。
// 可给 status 一个默认值，例如 NoteStatus::active。
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
    // 3. 新增笔记，分配 ID 并返回该 ID。
    long add(std::string title, std::string content) {
        const time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        const long id = next_id_++;
        notes_.push_back(Note{
            .id = id,
            // why: 这里需要使用std::move
            .title = std::move(title),
            .content = std::move(content),
            .create_time = now,
            .update_time = now});
        return id; // 替换为实际分配的 ID。
    }

    // 4. 以只读方式访问全部笔记。
    // const 引用避免复制整个 vector；调用者不能通过它修改集合。
    // why: 这里使用`[[nodiscard]]`+返回为引用
    [[nodiscard]] const std::vector<Note>& all() const {
        return notes_;
    }

    // 8. 按 ID 查找。
    // 下面是相关语法/API 的独立用法示例，不是本方法的实现：
    //
    // 1) std::find_if：传入一对迭代器和一个谓词，得到首个匹配元素的迭代器；
    //    找不到时结果等于 end()。需 #include <algorithm>。
    //    const auto it = std::find_if(items.begin(), items.end(),
    //        [](const auto& item) { return item.matches_condition(); });
    //    if (it != items.end()) { /* *it 是匹配元素 */ }
    //
    // 2) 裸指针作为“可能没有结果”的返回值：T* 可指向对象，也可为 nullptr。
    //    const T* result = nullptr;
    //    if (result != nullptr) { /* 通过 result 只读访问对象 */ }
    //    这种指针是借用，不拥有对象；容器改动可能使其失效。
    //
    // 3) std::optional<T> 表示“有一个 T 值或没有值”。需 #include <optional>。
    //    std::optional<int> result{42};
    //    if (result.has_value()) { const int value = result.value(); }
    //    std::optional<int> missing{}; // 空值；也可写 std::nullopt
    //    对 optional<Note> 来说，成功时通常保存 Note 的值副本。
    //
    // 请自行选择返回 const Note* 或 std::optional<Note>，再实现成员函数。
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

    // 9. 按 ID 删除，返回是否成功。
    // 下面只展示相关语法/API，不包含本项目的查找和删除逻辑：
    //
    // 1) 获取 vector 的迭代器：auto it = values.begin();
    //    迭代器像“指向容器元素的位置”，可用 *it 访问元素，++it 前进。
    //    循环结束位置用 values.end() 表示，end() 不是一个可解引用的元素。
    //
    // 2) vector::erase 接收要删除元素的迭代器，并返回删除位置之后的新迭代器：
    //    auto next = values.erase(it);
    //    删除会使被删位置及其后的相关迭代器/引用失效；之后应使用 erase
    //    返回的新迭代器或重新获取迭代器，不要继续使用旧 it。
    //
    // 3) bool 返回值通常用来表达操作是否成功：
    //    bool succeeded = false;
    //    if (succeeded) { /* 成功分支 */ } else { /* 未成功分支 */ }
    //
    // 请自行组合迭代器、查找方式和 erase，实现按 ID 删除并返回结果。
    auto delete_by_id(long id)
    {
        auto it = notes_.begin();
        while (it != notes_.end())
        {
            if (it->id == id)
            {
                notes_.erase(it);
                return true;
            }
            ++it;
        }
        return false;
    }

private:
    long next_id_{1};
    std::vector<Note> notes_;
};

// 5. 实现单条笔记的展示函数。
// 可以先接受 const Note&，并用 switch 根据状态输出可读文本。
static void display(const Note& note)
{
    switch (note.status)
    {
        default:
        case NoteStatus::active:
        // std::cout << note.id << ":" << note.title << " : " << note.content << std::endl;
        std::cout << std::format("{}: {}: {}", note.id, note.title, note.content) << std::endl;
        break;
        case NoteStatus::archived:
        //std::cout << "Archived note: "<< note.id << note.title << std::endl;
        std::cout << std::format("Archived note: {}: {}, archived time: {}", note.id, note.title, note.update_time) << std::endl;
        break;
    }
}

int main() {
    NoteStore store;

    // 6. 新增两条示例笔记，并输出 add 返回的 ID。
    // 示例：const int first_id = store.add("C++20", "Review value semantics");
    const long id_1 = store.add("title", "content");
    std::cout << id_1 << std::endl;
    const long id_2 = store.add("abc", "123");
    std::cout << id_2 << std::endl;
    const long id_3 = store.add("666", "你好");
    std::cout << id_3 << std::endl;

    // 7. 遍历 store.all() 并展示笔记。
    for (const Note& note : store.all())
    {
        display(note);
    }

    // 10. 删除
    if (store.delete_by_id(id_2))
    {
        std::cout << id_2 << " is deleted." << std::endl;
    }

    // 11. 删除后检查是否存在
    std::optional<Note> find_r2 = store.find_by_id(id_2);
    std::string res2 = find_r2 == std::nullopt ? "is nothing" : find_r2 -> title;
    std::cout << "id_2: " << res2 << std::endl;

    std::optional<Note> find_r3 = store.find_by_id(id_3);
    std::string res3 = find_r3 == std::nullopt ? "is nothing" : find_r3 -> title;
    std::cout << "id_3: " << res3 << std::endl;

    std::cout << "Stage 1 starter: complete the TODOs in src/main.cpp\n";
    return 0;
}
