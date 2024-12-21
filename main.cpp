#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <cassert>
#include <cstdio>
#include <climits>
#include <ctime>
#include <cstdint>
using namespace std;

#define DATA_DIRECTORY "./data/"
#define BASE_FILE_PATH (DATA_DIRECTORY "Base.txt")
#define USERS_FILE "users.txt"

#define MAX_NAME_LENGTH 50
#define MAX_USER_LENGTH 30
#define MAX_PASSWORD_LENGTH 30
#define MAX_HASHED_PASSWORD_LENGTH 100
#define MAX_FILENAME_LENGTH 50

typedef struct {
  int* answers;
  int answers_size;
  bool is_valid;
} Page;

typedef struct {
  string name;
  Page page;
  int score;
  int rank;
} Student;

typedef struct {
  int correct_answers;
  int wrong_answers;
  int no_answers;
  int difficulty;
} Question;

void clear_screen();
void print_header();
void print_menu_option(int n, const char* txt);
int get_number(int min, int max);
int get_positive_number(const char* prompt);
char* get_string(int size, const char* prompt);
void pause();

string my_hash(const char* txt);
bool login();
bool signup();

void first_menu();
Student* second_menu(int* n_students, Page* base);
void third_menu(Student* students, int n_students, const Page* base);

bool read_page_from_file(Student* s, const char* filename, const Page* base);
bool read_base_page(Page* p);
void read_students(Student* students, Page* base, int n);

void calc_score(Student* s, const Page* base);

void swap(Student* a, Student* b);
int get_highest_score_idx(const Student* students, int n);
void sort_students_by_score(Student* students, int n);

Question* analyze_questions(const Student* students, const Page* base, int n, int* n_qs);

const Student* get_student_by_name(const Student* students, int n, const char* name);

void print_page(const Page* page);
void print_student(const Student* s, bool show_page = false);
void print_students(const Student* students, int n);
void print_question(const Question* qs, int i);

bool export_as_txt(const Student* students, int n);
bool export_as_csv(const Student* students, int n);

int main() {
  first_menu();

  int n_students;
  Page base;
  Student* students = second_menu(&n_students, &base);
  
  third_menu(students, n_students, &base);
  return 0;
}

void first_menu() {
  bool success = false;
  while (!success) {
    clear_screen();
    print_header();
    print_menu_option(1, "Login");
    print_menu_option(2, "Sign-up");
    print_menu_option(0, "Exit");
    int n = get_number(0, 2);

    char msg[100];

    switch (n) {
      case 1:
        if (login()) {
          strcpy(msg, "Logged in successfully");
          success = true;
        } else {
          strcpy(msg, "Invalid Credentials");
          success = false;
        }
        break;
      case 2:
        if (signup()) {
          strcpy(msg, "Account has been created successfully");
          success = true;
        } else {
          strcpy(msg, "Failed to create new account");
          success = false;
        }
        break;
      case 0:
        exit(0);
      default:
        assert(0 && "UNREACHABLE");
    }

    cout << "\n >>> " <<  msg << "\n\n";
    pause();
  }
}

Student* second_menu(int* n_students, Page* base) {
  clear_screen();
  print_header();

  *n_students = get_positive_number("Enter number of students");

  Student* students = new Student[*n_students];
  read_students(students, base, *n_students);

  return students;
}

void third_menu(Student* students, int n_students, const Page* base) {
  sort_students_by_score(students, n_students);
  int n_qs;
  Question* qs = analyze_questions(students, base, n_students, &n_qs);

  while (1) {
    clear_screen();
    print_header();
    print_menu_option(1, "Top 3 students");
    print_menu_option(2, "Lookup student");
    print_menu_option(3, "Hardest question");
    print_menu_option(4, "Easiest question");
    print_menu_option(5, "Print all students");
    print_menu_option(6, "Print all questions");
    print_menu_option(7, "Export");
    print_menu_option(0, "Exit");
    int n = get_number(0, 7);

    switch (n) {
      case 1:
        for (int i = 0; students[i].page.is_valid && students[i].rank <= 3; i++)
          print_student(&students[i]);
        break;
      case 2: {
        cin.ignore();
        char* name = get_string(MAX_NAME_LENGTH, "Enter student name");
        const Student* s = get_student_by_name(students, n_students, name);
        if (!s)
          cout << "\n >>> Student not found\n";
        else
          print_student(s, true);
      }
        break;
      case 3: {
        int max = INT_MIN;
        int max_idx = -1;

        for (int i = 0; i < n_qs; i++)
          if (qs[i].difficulty > max) {
            max = qs[i].difficulty;
            max_idx = i;
          }

        print_question(qs, max_idx);
      }
        break;
      case 4: {
        int min = INT_MAX;
        int min_idx = -1;

        for (int i = 0; i < n_qs; i++)
          if (qs[i].difficulty < min) {
            min = qs[i].difficulty;
            min_idx = i;
          }

        print_question(qs, min_idx);
      }
        break;
      case 5:
        print_students(students, n_students);
        break;
      case 6:
        for (int i = 0; i < n_qs; i++)
          print_question(qs, i);
        break;
      case 7: {
        print_menu_option(1, "Export as txt");
        print_menu_option(2, "Export as csv");
        int n = get_number(1, 2);
        
        if (n == 1)
          export_as_txt(students, n_students);
        else if (n == 2)
          export_as_csv(students, n_students);
        else
          assert(0 && "UNREACHABLE");
      }
      break;
      case 0:
        exit(0);
      default:
        assert(0 && "UNREACHABLE");
    }

    pause();
  }
}

bool login() {
  clear_screen();
  print_header();

  cin.ignore();
  char* inp_user = get_string(MAX_USER_LENGTH, "Enter username");
  char* inp_pass = get_string(MAX_PASSWORD_LENGTH, "Enter password");

  ifstream file(USERS_FILE);

  if (!file) {
    cerr << "Failed to open \"" << USERS_FILE << "\": ";
    cerr << strerror(errno) << endl;

    return false;
  }

  char user[MAX_USER_LENGTH];
  char pass[MAX_HASHED_PASSWORD_LENGTH];

  while (!file.eof()) {
    file >> user;
    file >> pass;

    if (strcmp(user, inp_user) == 0 && strcmp(pass, my_hash(inp_pass).c_str()) == 0) {
      file.close();
      return true;
    }
  }
  file.close();
  return false;
}
bool signup() {
  clear_screen();
  print_header();

  cin.ignore();
  char* inp_user = get_string(MAX_USER_LENGTH, "Enter username");;
  char* inp_pass = get_string(MAX_PASSWORD_LENGTH, "Enter password");;

  if (strlen(inp_pass) == 0 || strlen(inp_pass) == 0) {
    cout << "\nInvalid username\n";
    return false;
  }

  for (size_t i = 0; i < strlen(inp_user); i++)
    if (!isalpha(inp_user[i])) {
      cout << "\nInvalid username\n";
      return false;
    }

  for (size_t i = 0; i < strlen(inp_pass); i++)
    if (!(isalnum(inp_pass[i]) || ispunct(inp_pass[i]))) {
      cout << "\nInvalid password\n";
      return false;
    }

  fstream file(USERS_FILE, ios::app | ios::in);

  if (!file) {
    cerr << "Failed to open \"" << USERS_FILE << "\": ";
    cerr << strerror(errno) << endl;

    return false;
  }

  file.seekg(0);

  char user[MAX_USER_LENGTH];
  char pass[MAX_HASHED_PASSWORD_LENGTH];

  while (!file.eof()) {
    file >> user;
    file >> pass;

    if (strcmp(user, inp_user) == 0) {
      file.close();
      return false;
    }
  }

  file.clear();
  file << inp_user << ' ' << my_hash(inp_pass) << '\n';
  file.flush();
  file.close();
  return true;
}

void print_header() {
  cout << 
  "    ______          __  ______            __\n"
  "   /_  __/__  _____/ /_/ ____/   ______ _/ /\n"
  "    / / / _ \\/ ___/ __/ __/ | | / / __ `/ /\n"
  "   / / /  __(__  ) /_/ /___ | |/ / /_/ / /\n"
  "  /_/  \\___/____/\\__/_____/ |___/\\__,_/_/\n";

  cout << "    A tool for evaluating and analyzing\n";
  cout << "         student test performance\n";
  cout << "\n  > GitHub: rzr8i/TestEval\n";
  cout << "----------------------------------------------\n\n";
}

void clear_screen() {
#ifdef _WIN32
  system("cls");
#else
  system("clear");
#endif
}

void pause() {
  cout << "\n Press Enter to continue...\n";
  getchar();
  cin.ignore();
}

int get_number(int min, int max) {
  int n;
  do {
    cout << "\n Enter a number between " << min << " and " << max;
    cout << "\n  > ";
    cin >> n;
  } while(n < min || n > max);
  return n;
}

int get_positive_number(const char* prompt) {
  int n;
  do {
    cout << "\n " << prompt;
    cout << "\n  > ";
    cin >> n;
  } while(n <= 0);
  return n;
}

char* get_string(int size, const char* prompt) {
  char* input = new char[size];

  cout << ' ' << prompt << ":\n";
  cout << "  > ";
  cin.getline(input, size);

  return input;
}

void print_menu_option(int n, const char* txt) {
  cout << "     (" << n << ") " << txt << "\n";
}

bool read_base_page(Page* p) {
  string filename = BASE_FILE_PATH;
  ifstream file(filename);

  if (!file) {
    cerr << "Failed to open \"" << filename << "\": ";
    cerr << strerror(errno) << endl;
    return false;
  }
  
  int lines = 0;
  int n;

  while (file >> n)
    lines++;

  file.clear();
  file.seekg(0);

  p->answers = new int[lines];
  p->answers_size = 0;
  while (file >> n)
    p->answers[p->answers_size++] = n;

  p->is_valid = true;

  file.close();
  return true;
}

bool read_page_from_file(Student* s, const char* filename, const Page* base) {
  ifstream file(filename);

  if (!file) {
    cerr << "Failed to open \"" << filename << "\": ";
    cerr << strerror(errno) << endl;

    s->page.is_valid = false;

    return false;
  }

  getline(file, s->name, '\n');

  int pos = file.tellg();
  int n;
  int lines = 0;

  while (file >> n)
    lines++;

  if (lines != base->answers_size) {
    s->page.answers_size = 0;
    s->page.is_valid = false;
    file.close();
    return false;
  }

  file.clear();
  file.seekg(pos);

  s->page.answers = new int[lines];
  s->page.answers_size = 0;

  while (file >> n)
    s->page.answers[s->page.answers_size++] = n;

  assert(s->page.answers_size == base->answers_size);
  s->page.is_valid = true;

  file.close();
  return true;
}

void calc_score(Student* s, const Page* base) {
  if (!s->page.is_valid) {
    s->score = INT_MIN;
    return;
  }
  assert(s->page.answers_size == base->answers_size);

  s->score = 0;
  int st_ans;

  for (int i = 0; i < base->answers_size; i++) {
    st_ans = s->page.answers[i];
    if (st_ans == 0) continue;

    s->score += st_ans == base->answers[i] ? 3 : -3;
  }
}

void print_page(const Page* page) {
  if (!page->is_valid) {
    cout << "Invalid Page\n";
    return;
  }

  for (int i = 0; i < page->answers_size; i++) {
    for (int j = 1; j <= 4; j++)
      cout << (int)(page->answers[i] == j) << ' ';
    cout << '\n';
  }
}

void print_student(const Student* s, bool show_page) {
  if (!s->page.is_valid) return;

  cout << s->name << " (Rank #" << s->rank << ")\n";
  if (show_page)
    print_page(&s->page);
  cout << "  Score: " << s->score << '\n';
}

void print_students(const Student* students, int n) {
  for (int i = 0; i < n; i++) {
    if (!students[i].page.is_valid) continue;
    print_student(&students[i]);
    cout << '\n';
  }
}

void print_question(const Question* qs, int i) {
  cout << "\n Question #" << i+1 << '\n';
  cout << "\tCorrect Answers: " << qs[i].correct_answers << '\n';
  cout << "\tWrong Answers: " << qs[i].wrong_answers << '\n';
  cout << "\tNo Answers: " << qs[i].no_answers << '\n';
}

void read_students(Student* students, Page* base, int n) {
  int digits = 0;
  int tmp = n;
  while (tmp) {
    digits++;
    tmp /= 10;
  }

  string base_path = DATA_DIRECTORY;
  char* filename = new char[4+digits+4+1];
  char* format = new char[20];

  sprintf(format, "file%%0%dd.txt", digits);

  assert(read_base_page(base));

  for (int i = 0; i < n; i++) {
    sprintf(filename, format, i+1);
    if (!read_page_from_file(&students[i], (base_path+filename).c_str(), base)) {
      students[i].page.is_valid = false;
    }
    calc_score(&students[i], base);
  }

  delete[] filename;
  delete[] format;

  sort_students_by_score(students, n);

  if (!students[0].page.is_valid)
    return;

  int rank = 1;
  int prev_score = students[0].score;

  for (int i = 0; i < n; i++)
    if (!students[i].page.is_valid)
      students[i].rank = -1;
    else {
      if (students[i].score == prev_score)
        students[i].rank = rank;
      else
        students[i].rank = ++rank;

      prev_score = students[i].score;
    }

}

void swap(Student* a, Student* b) {
  Student tmp = *a;
  *a = *b;
  *b = tmp;
}

int get_highest_score_idx(const Student* students, int n) {
  int highest_score = INT_MIN; 
  int highest_idx = -1;

  for (int i = 0; i < n; i++)
    if (students[i].score >= highest_score) {
      highest_score = students[i].score;
      highest_idx = i;
    }

  return highest_idx;
}

void sort_students_by_score(Student* students, int n) {
  int highest_idx;
  for (int i = 0; i < n-1; i++) {
    highest_idx = i + get_highest_score_idx(students+i, n-i);
    swap(students[i], students[highest_idx]);
  }
}

Question* analyze_questions(const Student* students, const Page* base, int n, int* n_qs) {
  *n_qs = base->answers_size;
  Question* qs = new Question[*n_qs];
  memset(qs, 0, *n_qs * sizeof(Question));

  const Page* c_page;

  for (int i = 0; i < *n_qs; i++)
    for (int j = 0; j < n; j++) {
      c_page = &students[j].page;
      if (!c_page->is_valid) continue;

      if (c_page->answers[i] == 0)
        qs[i].no_answers++;
      else if (c_page->answers[i] == base->answers[i])
        qs[i].correct_answers++;
      else
        qs[i].wrong_answers++;
    }


  for (int i = 0; i < *n_qs; i++)
    qs[i].difficulty = qs[i].wrong_answers + qs[i].no_answers - qs[i].correct_answers;

  return qs;
}

const Student* get_student_by_name(const Student* students, int n, const char* name) {
  for (int i = 0; i < n; i++) {
    if (!students[i].page.is_valid) continue;
    if (strcmp(students[i].name.c_str(), name) == 0)
      return &students[i];
  }

  return nullptr;
}

bool export_as_txt(const Student* students, int n) {
  time_t now = time(0);
  tm* time_info = localtime(&now);
  char filename[MAX_FILENAME_LENGTH];
  strftime(filename, sizeof(filename), "Students_%Y%m%d_%H%M%S.txt", time_info);

  ofstream f(filename);
  if (!f) {
    cerr << "Failed to open \"" << filename << "\": ";
    cerr << strerror(errno) << endl;
    return false;
  }

  for (int i = 0; i < n; i++) {
    if (!students[i].page.is_valid) continue;
    f << students[i].name << '\n';
    f << '\t' << "Score: " << students[i].score << '\n';
    f << '\t' << "Rank: " << students[i].rank << '\n';
    f << '\n';
  }

  cout << " File \"" << filename << "\" saved\n";

  f.close();
  return true;
}

bool export_as_csv(const Student* students, int n) {
  time_t now = time(0);
  tm* time_info = localtime(&now);
  char filename[MAX_FILENAME_LENGTH];
  strftime(filename, sizeof(filename), "Students_%Y%m%d_%H%M%S.csv", time_info);

  ofstream f(filename);
  if (!f) {
    cerr << "Failed to open \"" << filename << "\": ";
    cerr << strerror(errno) << endl;
    return false;
  }

  f << "Name,Score,Rank\n";

  for (int i = 0; i < n; i++) {
    if (!students[i].page.is_valid) continue;
    f << students[i].name << ',' << students[i].score << ',' <<  students[i].rank << '\n';
  }

  cout << " File \"" << filename << "\" saved\n";

  f.close();
  return true;
}

string my_hash(const char* txt) {
  uint32_t o[] = {
    0x68212253, 0x6a232448, 0x60252639, 0x5a272826,
    0x7d292a4d, 0x5c2b2c38, 0x7b2d2e78, 0x492f3a35,
    0x273b3c7c, 0x713d3e3e, 0x773f403f, 0x635b5c4b,
    0x515d5e5d, 0x455f605f, 0x4a7b7c55, 0x3d7d7e59,
    0x62293032, 0x2a313221, 0x72333425, 0x6135365e,
    0x3137384e, 0x3c394175, 0x4042436c, 0x7a444544,
    0x33464741, 0x2948496d, 0x224a4b42, 0x654c4d3a,
    0x2f4e4f52, 0x2850516f, 0x73525354, 0x36545529,
    0x43565769, 0x5b585950, 0x6b5a6123, 0x5862632b,
    0x56646567, 0x2c66674f, 0x6668692d, 0x306a6b79,
    0x476c6d74, 0x466e6f24, 0x64707157, 0x70727337,
    0x3474754c, 0x3b767776, 0x2e78797e, 0x6e7a0a0d
  };
  stringstream r;
  int len = strlen(txt);
  for (int i = 0; i < len; i++) {
    uint32_t *j = o;
    while (1) {
      for (int k = 16; k > 4; k>>=1)
        if (!(*(txt+i) - ((*j>>k)&0xff)))
          r << setfill('0') << setw(3) << hex << (((*j>>((k-8)*3))&0xff)-0x21)*len;

      if ((*(j++)&0xff) == '\r')
        break;
    }
  }
  return r.str();
}
