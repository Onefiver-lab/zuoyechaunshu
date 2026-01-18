export module registrar:datamanager_postgres;
import :datamanager_interface;
import :entity; // 仅导入公共模块
import :secretary;
import pqxx;
import std;

using std::format;
using std::move;

export class DataManagerPostgres : public DataManager {
public:
    DataManagerPostgres(const std::string& connStr) {
        try {
            m_conn = std::make_unique<pqxx::connection>(connStr);
            if (m_conn->is_open()) {
                std::cout << "Connected to database: " << m_conn->dbname() << std::endl;
            } else {
                throw std::runtime_error("Failed to connect to database");
            }
        } catch (const std::exception& e) {
            std::cerr << "Database connection error: " << e.what() << std::endl;
            throw;
        }
    }

    // 学生操作
    void addStudent(SharedStudent student) override {
        if (getStudentById(student->getId())) return;
        try {
            pqxx::work txn(*m_conn);
            std::string sql = format(
                "INSERT INTO students (id, name) VALUES ('{}', '{}')",
                txn.esc(student->getId()), txn.esc(student->getName())
            );
            txn.exec(sql);
            txn.commit();
        } catch (const std::exception& e) {
            std::cerr << "Add student error: " << e.what() << std::endl;
        }
    }

    SharedStudent getStudentById(std::string id) override {
        try {
            pqxx::nontransaction txn(*m_conn);
            std::string sql = format("SELECT name FROM students WHERE id = '{}'", txn.esc(id));
            pqxx::result res = txn.exec(sql);
            if (res.empty()) return nullptr;
            return std::make_shared<Student>(id, res[0][0].as<std::string>());
        } catch (const std::exception& e) {
            std::cerr << "Get student error: " << e.what() << std::endl;
            return nullptr;
        }
    }

    StudentList getAllStudents() override {
        StudentList students;
        try {
            pqxx::nontransaction txn(*m_conn);
            pqxx::result res = txn.exec("SELECT id, name FROM students");
            for (auto row : res) {
                students.push_back(std::make_shared<Student>(row[0].as<std::string>(), row[1].as<std::string>()));
            }
        } catch (const std::exception& e) {
            std::cerr << "Get all students error: " << e.what() << std::endl;
        }
        return students;
    }

    // 课程操作
    void addCourse(SharedCourse course) override {
        if (getCourseById(course->getId())) return;
        try {
            pqxx::work txn(*m_conn);
            std::string teacherId = course->getTeacher() ? course->getTeacher()->getId() : "";
            std::string sql = format(
                "INSERT INTO courses (id, name, credits, teacher_id) VALUES ('{}', '{}', {}, '{}')",
                txn.esc(course->getId()), txn.esc(course->getName()), course->getCredits(), txn.esc(teacherId)
            );
            txn.exec(sql);

            // 同步学生选课数据
            for (auto stu : course->getStudents()) {
                std::string gradeSql = format(
                    "INSERT INTO student_course (student_id, course_id, grade) VALUES ('{}', '{}', '{}')",
                    txn.esc(stu->getId()), txn.esc(course->getId()), txn.esc(course->getGrade(stu))
                );
                txn.exec(gradeSql);
            }
            txn.commit();
        } catch (const std::exception& e) {
            std::cerr << "Add course error: " << e.what() << std::endl;
        }
    }

    SharedCourse getCourseById(std::string id) override {
        try {
            pqxx::nontransaction txn(*m_conn);
            std::string sql = format("SELECT name, credits, teacher_id FROM courses WHERE id = '{}'", txn.esc(id));
            pqxx::result res = txn.exec(sql);
            if (res.empty()) return nullptr;

            auto course = std::make_shared<Course>(id, res[0][0].as<std::string>(), res[0][1].as<int>());
            std::string teacherId = res[0][2].as<std::string>();
            if (!teacherId.empty()) {
                auto tea = getTeacherById(teacherId);
                if (tea) course->setTeacher(tea);
            }

            // 关联学生和成绩
            std::string scSql = format(
                "SELECT student_id, grade FROM student_course WHERE course_id = '{}'", txn.esc(id)
            );
            pqxx::result scRes = txn.exec(scSql);
            for (auto row : scRes) {
                auto stu = getStudentById(row[0].as<std::string>());
                if (stu) {
                    course->addStudent(stu);
                    course->assignGrade(stu, row[1].as<std::string>());
                }
            }
            return course;
        } catch (const std::exception& e) {
            std::cerr << "Get course error: " << e.what() << std::endl;
            return nullptr;
        }
    }

    CourseList getAllCourses() override {
        CourseList courses;
        try {
            pqxx::nontransaction txn(*m_conn);
            pqxx::result res = txn.exec("SELECT id FROM courses");
            for (auto row : res) {
                auto course = getCourseById(row[0].as<std::string>());
                if (course) courses.push_back(course);
            }
        } catch (const std::exception& e) {
            std::cerr << "Get all courses error: " << e.what() << std::endl;
        }
        return courses;
    }

    // 教师操作
    void addTeacher(SharedTeacher teacher) override {
        if (getTeacherById(teacher->getId())) return;
        try {
            pqxx::work txn(*m_conn);
            std::string sql = format(
                "INSERT INTO teachers (id, name) VALUES ('{}', '{}')",
                txn.esc(teacher->getId()), txn.esc(teacher->getName())
            );
            txn.exec(sql);
            txn.commit();
        } catch (const std::exception& e) {
            std::cerr << "Add teacher error: " << e.what() << std::endl;
        }
    }

    SharedTeacher getTeacherById(std::string id) override {
        try {
            pqxx::nontransaction txn(*m_conn);
            std::string sql = format("SELECT name FROM teachers WHERE id = '{}'", txn.esc(id));
            pqxx::result res = txn.exec(sql);
            if (res.empty()) return nullptr;
            return std::make_shared<Teacher>(id, res[0][0].as<std::string>());
        } catch (const std::exception& e) {
            std::cerr << "Get teacher error: " << e.what() << std::endl;
            return nullptr;
        }
    }

    std::vector<SharedTeacher> getAllTeachers() override {
        std::vector<SharedTeacher> teachers;
        try {
            pqxx::nontransaction txn(*m_conn);
            pqxx::result res = txn.exec("SELECT id, name FROM teachers");
            for (auto row : res) {
                teachers.push_back(std::make_shared<Teacher>(row[0].as<std::string>(), row[1].as<std::string>()));
            }
        } catch (const std::exception& e) {
            std::cerr << "Get all teachers error: " << e.what() << std::endl;
        }
        return teachers;
    }

    // 秘书操作
    void addSecretary(SharedSecretary secretary) override {
        if (getSecretaryById(secretary->getId())) return;
        try {
            pqxx::work txn(*m_conn);
            std::string sql = format(
                "INSERT INTO secretaries (id, name) VALUES ('{}', '{}')",
                txn.esc(secretary->getId()), txn.esc(secretary->getName())
            );
            txn.exec(sql);
            txn.commit();
        } catch (const std::exception& e) {
            std::cerr << "Add secretary error: " << e.what() << std::endl;
        }
    }

    SharedSecretary getSecretaryById(std::string id) override {
        try {
            pqxx::nontransaction txn(*m_conn);
            std::string sql = format("SELECT name FROM secretaries WHERE id = '{}'", txn.esc(id));
            pqxx::result res = txn.exec(sql);
            if (res.empty()) return nullptr;
            return std::make_shared<Secretary>(id, res[0][0].as<std::string>());
        } catch (const std::exception& e) {
            std::cerr << "Get secretary error: " << e.what() << std::endl;
            return nullptr;
        }
    }

    bool saveData() override {
        return m_conn->is_open();
    }

private:
    std::unique_ptr<pqxx::connection> m_conn;
};
