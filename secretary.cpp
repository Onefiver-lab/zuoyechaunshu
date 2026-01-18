export module registrar:secretary;
import :entity; // 仅导入公共模块
import std;

using std::format;
using std::make_shared;

export class Secretary : public BaseEntity {
public:
    Secretary(std::string id, std::string name) : BaseEntity(std::move(id), std::move(name)) {}

    std::string info() const override {
        return format("秘书ID: {}, 姓名: {}", getId(), getName());
    }

    SharedCourse createCourse(std::string courseId, std::string courseName, int credits) {
        return make_shared<Course>(std::move(courseId), std::move(courseName), credits);
    }

    bool assignTeacherToCourse(SharedTeacher teacher, SharedCourse course) {
        if (!teacher || !course) return false;
        course->setTeacher(teacher);
        return true;
    }
};
