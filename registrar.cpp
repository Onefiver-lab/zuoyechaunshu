export module registrar;
export import :entity; // 导出公共模块，供main使用
export import :student;
export import :course;
export import :teacher;
export import :secretary;
export import :datamanager_interface;
export import :datamanager_memory;
import std;

using std::string;
using std::vector;
using std::cout;
using std::cin;
using std::endl;
using std::find;
using std::format;
using std::make_shared;
using std::numeric_limits;
using std::streamsize;
using std::invalid_argument;

// 工具函数：安全读取整数
int readInt(const string& prompt) {
    int val;
    while (true) {
        cout << prompt;
        if (cin >> val) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return val;
        } else {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "输入无效，请输入数字！" << endl;
        }
    }
}

export class Registrar {
public:
    Registrar() {
        try {
#ifdef USE_POSTGRES
            string connStr = "user=postgres password=123456 dbname=course_selection host=localhost port=5432";
            m_dataManager = make_shared<DataManagerPostgres>(connStr);
#else
            m_dataManager = make_shared<DataManagerMemory>();
#endif
            initDefaultUsers();
        } catch (const std::exception& e) {
            cout << "系统初始化失败：" << e.what() << endl;
            exit(1);
        }
    }

    int exec();

private:
    std::shared_ptr<DataManager> m_dataManager;
    void initDefaultUsers();
    void studentMenu();
    void teacherMenu();
    void secretaryMenu();
    void showAllCourses();
    void enrollCourse(SharedStudent student);
    void showStudentGrades(SharedStudent student);
    void showTeacherCourses(SharedTeacher teacher);
    void inputStudentGrades(SharedTeacher teacher);
    void createCourseBySecretary(SharedSecretary secretary);
    void assignTeacherBySecretary(SharedSecretary secretary);
    void showAllTeachers();
};

// 系统主循环
int Registrar::exec() {
    cout << "===== 选课系统 =====" << endl;
    while (true) {
        int choice = readInt("\n1.学生 2.教师 3.教学秘书 0.退出\n请选择：");
        switch (choice) {
            case 1: studentMenu(); break;
            case 2: teacherMenu(); break;
            case 3: secretaryMenu(); break;
            case 0: cout << "再见！" << endl; return 0;
            default: cout << "无效选项！" << endl;
        }
    }
}

// 初始化默认测试用户
void Registrar::initDefaultUsers() {
    try {
        // 学生
        if (!m_dataManager->getStudentById("2024001")) {
            m_dataManager->addStudent(make_shared<Student>("2024001", "张三"));
            m_dataManager->addStudent(make_shared<Student>("2024002", "李四"));
        }

        // 教师
        if (!m_dataManager->getTeacherById("T001")) {
            m_dataManager->addTeacher(make_shared<Teacher>("T001", "王老师"));
            m_dataManager->addTeacher(make_shared<Teacher>("T002", "李老师"));
        }

        // 秘书
        if (!m_dataManager->getSecretaryById("S001")) {
            m_dataManager->addSecretary(make_shared<Secretary>("S001", "赵秘书"));
        }

        // 课程
        if (!m_dataManager->getCourseById("C001")) {
            auto sec = m_dataManager->getSecretaryById("S001");
            auto tea = m_dataManager->getTeacherById("T001");
            auto course = sec->createCourse("C001", "C++程序设计", 3);
            sec->assignTeacherToCourse(tea, course);
            m_dataManager->addCourse(course);
        }

        if (!m_dataManager->saveData()) {
            cout << "默认数据保存失败！" << endl;
        }
    } catch (const std::exception& e) {
        cout << "默认用户初始化失败：" << e.what() << endl;
    }
}

// 学生菜单
void Registrar::studentMenu() {
    cout << "\n===== 学生登录 =====" << endl;
    cout << "输入学生ID：";
    string id;
    getline(cin, id);
    auto stu = m_dataManager->getStudentById(id);
    if (!stu) { cout << "ID不存在！" << endl; return; }

    while (true) {
        int choice = readInt("\n1.查课程 2.选课 3.课表 4.成绩 0.返回\n选择：");
        switch (choice) {
            case 1: showAllCourses(); break;
            case 2: enrollCourse(stu); break;
            case 3: cout << "个人课表：\n" << stu->schedule(); break;
            case 4: showStudentGrades(stu); break;
            case 0: return;
            default: cout << "无效选项！" << endl;
        }
    }
}

// 教师菜单
void Registrar::teacherMenu() {
    cout << "\n===== 教师登录 =====" << endl;
    cout << "输入教师ID：";
    string id;
    getline(cin, id);
    auto tea = m_dataManager->getTeacherById(id);
    if (!tea) { cout << "ID不存在！" << endl; return; }

    while (true) {
        int choice = readInt("\n1.授课课程 2.录成绩 0.返回\n选择：");
        switch (choice) {
            case 1: showTeacherCourses(tea); break;
            case 2: inputStudentGrades(tea); break;
            case 0: return;
            default: cout << "无效选项！" << endl;
        }
    }
}

// 秘书菜单
void Registrar::secretaryMenu() {
    cout << "\n===== 秘书登录 =====" << endl;
    cout << "输入秘书ID：";
    string id;
    getline(cin, id);
    auto sec = m_dataManager->getSecretaryById(id);
    if (!sec) { cout << "ID不存在！" << endl; return; }

    while (true) {
        int choice = readInt("\n1.创建课程 2.分配教师 3.查课程 0.返回\n选择：");
        switch (choice) {
            case 1: createCourseBySecretary(sec); break;
            case 2: assignTeacherBySecretary(sec); break;
            case 3: showAllCourses(); break;
            case 0: return;
            default: cout << "无效选项！" << endl;
        }
    }
}

// 显示所有课程
void Registrar::showAllCourses() {
    cout << "\n===== 所有课程 =====" << endl;
    auto courses = m_dataManager->getAllCourses();
    if (courses.empty()) { cout << "暂无课程！" << endl; return; }
    for (const auto& c : courses) cout << c->info();
}

// 学生选课
void Registrar::enrollCourse(SharedStudent stu) {
    showAllCourses();
    cout << "输入选课ID：";
    string cid;
    getline(cin, cid);
    auto course = m_dataManager->getCourseById(cid);
    if (!course) { cout << "课程不存在！" << endl; return; }

    try {
        course->addStudent(stu);
        m_dataManager->addCourse(course);
        if (m_dataManager->saveData()) {
            cout << "选课成功！" << endl;
        } else {
            cout << "选课成功，但数据保存失败！" << endl;
        }
    } catch (const std::exception& e) {
        cout << "选课失败：" << e.what() << endl;
    }
}

// 显示学生成绩
void Registrar::showStudentGrades(SharedStudent stu) {
    cout << "\n===== 个人成绩 =====" << endl;
    auto courses = m_dataManager->getAllCourses();
    bool hasGrade = false;
    for (const auto& c : courses) {
        auto& stus = c->getStudents();
        if (find(stus.begin(), stus.end(), stu) != stus.end()) {
            cout << format("{}: 成绩{}", c->getName(), c->getGrade(stu)) << endl;
            hasGrade = true;
        }
    }
    if (!hasGrade) cout << "暂无成绩！" << endl;
}

// 显示教师授课课程
void Registrar::showTeacherCourses(SharedTeacher tea) {
    cout << "\n===== 授课课程 =====" << endl;
    auto courses = m_dataManager->getAllCourses();
    bool hasCourse = false;
    for (const auto& c : courses) {
        if (c->getTeacher() == tea) {
            cout << c->info();
            hasCourse = true;
        }
    }
    if (!hasCourse) cout << "暂无授课课程！" << endl;
}

// 教师录成绩
void Registrar::inputStudentGrades(SharedTeacher tea) {
    auto courses = m_dataManager->getAllCourses();
    vector<SharedCourse> myCourses;
    for (const auto& c : courses) {
        if (c->getTeacher() == tea) myCourses.push_back(c);
    }
    if (myCourses.empty()) { cout << "无授课课程！" << endl; return; }

    cout << "授课课程：" << endl;
    for (size_t i = 0; i < myCourses.size(); i++)
        cout << format("{}: {}", i + 1, myCourses[i]->getName()) << endl;

    int idx = readInt("选择课程序号：");
    if (idx < 1 || idx > myCourses.size()) { cout << "无效序号！" << endl; return; }
    auto course = myCourses[idx - 1];

    auto stus = course->getStudents();
    if (stus.empty()) { cout << "无学生选课！" << endl; return; }

    cout << "选课学生：" << endl;
    for (size_t i = 0; i < stus.size(); i++)
        cout << format("{}: {}", i + 1, stus[i]->info());

    int stuIdx = readInt("选择学生序号：");
    if (stuIdx < 1 || stuIdx > stus.size()) { cout << "无效序号！" << endl; return; }
    auto student = stus[stuIdx - 1];

    cout << "输入成绩（支持A-F或0-100）：";
    string grade;
    getline(cin, grade);

    if (course->assignGrade(student, grade)) {
        m_dataManager->addCourse(course);
        if (m_dataManager->saveData()) {
            cout << "成绩录入成功！" << endl;
        } else {
            cout << "成绩录入成功，但数据保存失败！" << endl;
        }
    } else {
        cout << "成绩格式无效（仅支持A-F或0-100）！" << endl;
    }
}

// 秘书创建课程
void Registrar::createCourseBySecretary(SharedSecretary secretary) {
    cout << "输入课程ID：";
    string cid;
    getline(cin, cid);
    if (m_dataManager->getCourseById(cid)) {
        cout << "课程ID已存在！" << endl;
        return;
    }

    cout << "输入课程名称：";
    string cname;
    getline(cin, cname);
    int credits = readInt("输入课程学分（1-10）：");

    try {
        auto course = secretary->createCourse(cid, cname, credits);
        m_dataManager->addCourse(course);
        if (m_dataManager->saveData()) {
            cout << "课程创建成功！" << endl;
        } else {
            cout << "课程创建成功，但数据保存失败！" << endl;
        }
    } catch (const invalid_argument& e) {
        cout << "课程创建失败：" << e.what() << endl;
    }
}

// 秘书分配教师
void Registrar::assignTeacherBySecretary(SharedSecretary secretary) {
    showAllCourses();
    cout << "输入要分配教师的课程ID：";
    string cid;
    getline(cin, cid);
    auto course = m_dataManager->getCourseById(cid);
    if (!course) { cout << "课程不存在！" << endl; return; }

    showAllTeachers();
    cout << "输入教师ID：";
    string tid;
    getline(cin, tid);
    auto teacher = m_dataManager->getTeacherById(tid);
    if (!teacher) { cout << "教师不存在！" << endl; return; }

    if (secretary->assignTeacherToCourse(teacher, course)) {
        m_dataManager->addCourse(course);
        if (m_dataManager->saveData()) {
            cout << "教师分配成功！" << endl;
        } else {
            cout << "教师分配成功，但数据保存失败！" << endl;
        }
    } else {
        cout << "教师分配失败！" << endl;
    }
}

// 显示所有教师
void Registrar::showAllTeachers() {
    cout << "\n===== 所有教师 =====" << endl;
    auto teachers = m_dataManager->getAllTeachers();
    if (teachers.empty()) { cout << "暂无教师！" << endl; return; }
    for (const auto& t : teachers) cout << t->info();
}
