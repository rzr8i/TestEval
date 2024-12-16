#include <iostream>
using namespace std;

typedef struct {
	char* name;
	int* answers;
	int answers_size;
	int score;
} Page;

bool read_page_from_file(Page* page, const char* filename);
bool read_pages(Page** pages, int n);

int calc_score(Page* page, Page* base);
void sort_pages_by_score(Page** pages, int num_of_pages);

int get_hardest_question(Page** pages, int num_of_pages);
int get_easiest_question(Page** pages, int num_of_pages);

void print_page(Page* page);
void print_pages(Page** pages, int num_of_pages);

bool export_as_txt(Page** pages, int num_of_pages);
bool export_as_csv(Page** pages, int num_of_pages);

int main() {
	return 0;
}
