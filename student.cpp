export module registrar:student;
import :entity; // 仅导入公共模块
import std;

using std::format;
using std::find;

export class Student : public BaseEntity {
public:
    Student(std::string id, std::string name) : BaseEntity(std::move(id), std::move(name)) {}

    std::string info() const override {
        return format("学生ID: {}, 姓名: {}\n", getId(), getName());
    }

    void enrollIn(SharedCourse course) {
        auto it = find(_courses.begin(), _courses.end(), course);
        if (it != _courses.end()) return;
        _courses.push_back(course);
    }

    std::string schedule() {
        std::string scheduleStr;
        for (const auto& course : _courses) {
            scheduleStr += course->info();
        }
        return scheduleStr.empty() ? "暂无选课\n" : scheduleStr;
    }

private:
    CourseList _courses;
};
