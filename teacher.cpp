export module registrar:teacher;
import :entity; // 仅导入公共模块
import std;

using std::format;
using std::find;

export class Teacher : public BaseEntity {
public:
    Teacher(std::string id, std::string name) : BaseEntity(std::move(id), std::move(name)) {}

    std::string info() const override {
        return format("教师ID: {}, 姓名: {}", getId(), getName());
    }

    void addCourse(SharedCourse course) {
        auto it = find(m_courses.begin(), m_courses.end(), course);
        if (it != m_courses.end()) return;
        m_courses.push_back(course);
    }

    bool gradeStudent(SharedCourse course, SharedStudent student, std::string grade) {
        if (course->getTeacher() != this) return false;
        auto& students = course->getStudents();
        if (find(students.begin(), students.end(), student) == students.end()) return false;
        return course->assignGrade(student, std::move(grade));
    }

    CourseList getCourses() const { return m_courses; }

private:
    CourseList m_courses;
};
