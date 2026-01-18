export module registrar:course;
import :entity; // 仅导入公共模块
import std;

using std::format;
using std::map;
using std::find;

// 继承公共基类BaseEntity
export class Course : public BaseEntity, public std::enable_shared_from_this<Course> {
public:
    Course(std::string id, std::string name, int credits)
        : BaseEntity(std::move(id), std::move(name)), m_credits(credits) {
        if (credits < 1 || credits > 10) {
            throw std::invalid_argument("学分必须在1-10之间");
        }
    }

    std::string info() const override {
        std::string teacherInfo = m_teacher ? m_teacher->info() : "无授课教师";
        return format("课程ID: {}, 名称: {}, 学分: {}, 授课教师: {}\n",
                      getId(), getName(), m_credits, teacherInfo);
    }

    void addStudent(SharedStudent student) {
        auto it = find(m_students.begin(), m_students.end(), student);
        if (it != m_students.end()) return;
        m_students.push_back(student);
        student->enrollIn(shared_from_this());
    }

    void setTeacher(SharedTeacher teacher) {
        if (!teacher) return;
        m_teacher = teacher;
        teacher->addCourse(shared_from_this());
    }

    bool assignGrade(SharedStudent student, std::string grade) {
        if (!isValidGrade(grade)) return false;
        auto it = find(m_students.begin(), m_students.end(), student);
        if (it == m_students.end()) return false;
        m_grades[student] = std::move(grade);
        return true;
    }

    std::string getGrade(SharedStudent student) const {
        auto it = m_grades.find(student);
        return it != m_grades.end() ? it->second : "未评定";
    }

    SharedTeacher getTeacher() const { return m_teacher; }
    StudentList getStudents() const { return m_students; }
    int getCredits() const { return m_credits; }

private:
    int m_credits;
    SharedTeacher m_teacher{nullptr};
    StudentList m_students;
    map<SharedStudent, std::string> m_grades;
};
