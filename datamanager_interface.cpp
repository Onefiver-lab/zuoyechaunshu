export module registrar:datamanager_interface;
import :entity; // 仅导入公共模块
import std;

// 前向声明秘书类（无需完整导入模块）
export class Secretary;
using SharedSecretary = std::shared_ptr<Secretary>;

export class DataManager {
public:
    virtual ~DataManager() = default;

    // 学生操作
    virtual void addStudent(SharedStudent student) = 0;
    virtual SharedStudent getStudentById(std::string id) = 0;
    virtual StudentList getAllStudents() = 0;

    // 课程操作
    virtual void addCourse(SharedCourse course) = 0;
    virtual SharedCourse getCourseById(std::string id) = 0;
    virtual CourseList getAllCourses() = 0;

    // 教师操作
    virtual void addTeacher(SharedTeacher teacher) = 0;
    virtual SharedTeacher getTeacherById(std::string id) = 0;
    virtual std::vector<SharedTeacher> getAllTeachers() = 0;

    // 秘书操作
    virtual void addSecretary(SharedSecretary secretary) = 0;
    virtual SharedSecretary getSecretaryById(std::string id) = 0;

    // 数据持久化
    virtual bool saveData() = 0;
};
