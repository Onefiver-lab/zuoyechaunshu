// entity.ixx（公共模块接口，导出共享类型）
export module registrar:entity;
import std;

// 基础实体基类（所有业务实体的公共父类）
export class BaseEntity {
public:
    BaseEntity(std::string id, std::string name) : m_id(std::move(id)), m_name(std::move(name)) {}
    virtual ~BaseEntity() = default;

    std::string getId() const { return m_id; }
    std::string getName() const { return m_name; }
    bool hasId(const std::string& id) const { return m_id == id; }
    virtual std::string info() const = 0; // 纯虚函数，强制子类实现信息输出

private:
    std::string m_id;
    std::string m_name;
};

// 前向声明（解决交叉依赖，避免导入完整模块）
export class Student;
export class Teacher;
export class Course;

// 共享工具类型定义
export using SharedStudent = std::shared_ptr<Student>;
export using SharedTeacher = std::shared_ptr<Teacher>;
export using SharedCourse = std::shared_ptr<Course>;
export using StudentList = std::vector<SharedStudent>;
export using CourseList = std::vector<SharedCourse>;

// 成绩校验工具函数（共享逻辑）
export bool isValidGrade(const std::string& grade) {
    if (grade == "A" || grade == "B" || grade == "C" || grade == "D" || grade == "F") {
        return true;
    }
    try {
        int score = std::stoi(grade);
        return score >= 0 && score <= 100;
    } catch (...) {
        return false;
    }
}
