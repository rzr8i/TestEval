#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <cstdio>
#include <climits>
using namespace std;

typedef struct {
	int* answers;
	int answers_size;
	bool is_valid;
} Page;

typedef struct {
	string name;
	Page page;
	int score;
} Student;

typedef struct {
	int correct_answers;
	int wrong_answers;
	int no_answers;
	int difficulty;
} Question;

bool read_page_from_file(Student* s, const char* filename, const Page* base);
bool read_base_page(Page* p);
void read_students(Student* students, Page* base, int n);

void calc_score(Student* s, const Page* base);

void swap(Student* a, Student* b);
int get_highest_score_idx(const Student* students, int n);
void sort_students_by_score(Student* students, int n);

Question* analyze_questions(const Student* students, const Page* base, int n, int* n_qs);

void print_page(const Page* page);
void print_student(const Student* s);
void print_students(const Student* students, int n);

bool export_as_txt(const Student* students, int n);
bool export_as_csv(const Student* students, int n);

int main() {
	int n = 50;
	Student* students = new Student[n];
	Page base;
	read_students(students, &base, n);

	print_students(students, n);
	cout << "SORTING...\n";
	sort_students_by_score(students, n);
	print_students(students, n);


	int n_qs;
	Question* qs = analyze_questions(students, &base, n, &n_qs);

	for (int i = 0; i < n_qs; i++) {
		cout << "Question #" << i+1 << '\n';
		cout << "\tCorrect Answers: " << qs[i].correct_answers << '\n';
		cout << "\tWrong Answers: " << qs[i].wrong_answers << '\n';
		cout << "\tNo Answers: " << qs[i].no_answers << '\n';
	}

	delete[] qs;
	return 0;
}

bool read_base_page(Page* p) {
	string filename = "./data/base.txt";
	ifstream file(filename);

	if (!file) {
		cerr << "Failed to open \"" << filename << "\": ";
		cerr << strerror(errno) << endl;
		return false;
	}

	p->answers = new int[100];
	p->answers_size = 0;
	int n;
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
		return false;
	}

	getline(file, s->name, '\n');

	s->page.answers = new int[100];
	s->page.answers_size = 0;
	int n;

	while (file >> n)
		s->page.answers[s->page.answers_size++] = n;

	s->page.is_valid = s->page.answers_size == base->answers_size;
	if (!s->page.is_valid) {
		delete[] s->page.answers;
		s->page.answers_size = 0;
	}

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

void print_student(const Student* s) {
	cout << s->name << '\n';
	print_page(&s->page);
	if (s->page.is_valid)
		cout << "Score: " << s->score << '\n';
}

void print_students(const Student* students, int n) {
	for (int i = 0; i < n; i++) {
		print_student(&students[i]);
		cout << '\n';
	}
}

void read_students(Student* students, Page* base, int n) {
	int digits = 0;
	int tmp = n;
	while (tmp) {
		digits++;
		tmp /= 10;
	}

	string base_path = "./data/";
	char* filename = new char[4+digits+4+1];
	char* format = new char[20];

	sprintf(format, "file%%0%dd.txt", digits);

	assert(read_base_page(base));

	for (int i = 0; i < n; i++) {
		sprintf(filename, format, i+1);
		assert(read_page_from_file(&students[i], (base_path+filename).c_str(), base));
		calc_score(&students[i], base);
	}

	delete[] filename;
	delete[] format;
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
