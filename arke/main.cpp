#include <iostream>
#include <iomanip>
#include <memory>

using namespace std;

class grid {
	struct cell {
		int x;
		int y;
		int value;
		int neighbor_count;
		cell* neighbors[4];
	};

	std::unique_ptr<cell[]> cells;
	int length;

	cell* find_start();
	cell* get_cell(int x, int y);

	public:
		void print();
		bool fill(std::istream& source);
		bool find_path(cell* current_location = nullptr, cell* last_location = nullptr);
};

void grid::print() {
	cout << endl;

	for (int i = 0; i < this->length; i++) {
		for (int j = 0; j < this->length; j++) {
			cout << setw(2) << this->get_cell(j, i)->value << " ";
		}
		cout << endl;
	}

	cout << endl;
}

bool grid::fill(istream& source) {
	source >> this->length;

	if (this->length < 1)
		return false;

	this->cells = make_unique<cell[]>(length * length);

	for (int i = 0; i < length; i++) {
		for (int j = 0; j < length; j++) {
			auto current = this->get_cell(i, j);
			current->x = i;
			current->y = j;
			current->value = 0;
			current->neighbor_count = 0;

			auto current_neighbor = this->get_cell(i + 1, j);
			if (current_neighbor)
				current->neighbors[current->neighbor_count++] = current_neighbor;

			current_neighbor = this->get_cell(i - 1, j);
			if (current_neighbor)
				current->neighbors[current->neighbor_count++] = current_neighbor;

			current_neighbor = this->get_cell(i, j + 1);
			if (current_neighbor)
				current->neighbors[current->neighbor_count++] = current_neighbor;

			current_neighbor = this->get_cell(i, j - 1);
			if (current_neighbor)
				current->neighbors[current->neighbor_count++] = current_neighbor;
		}
	}

	for (int i = 0; i < this->length; i++)
		source >> this->get_cell(i, 0)->value;

	for (int i = 1; i < this->length - 1; i++)
		source >> this->get_cell(this->length - 1, i)->value;

	for (int i = this->length - 1; i >= 0; i--)
		source >> this->get_cell(i, this->length - 1)->value;

	for (int i = this->length - 2; i > 0; i--)
		source >> this->get_cell(0, i)->value;

	return true;
}

bool grid::find_path(cell* current_location, cell* last_location) {
	if (!current_location)
		current_location = this->find_start();

	if (current_location->value == this->length * this->length)
		return true;

	for (int i = 0; i < current_location->neighbor_count; i++)
	if (current_location->neighbors[i]->value == current_location->value + 1)
		return this->find_path(current_location->neighbors[i]);

	for (int i = 0; i < current_location->neighbor_count; i++) {
		auto neighbor = current_location->neighbors[i];

		if (neighbor != last_location && neighbor->value == 0) {
			neighbor->value = current_location->value + 1;

			if (this->find_path(neighbor, current_location))
				return true;

			neighbor->value = 0;
		}
	}

	return false;
}

grid::cell* grid::find_start() {
	for (int i = 0; i < this->length; i++)
	for (int j = 0; j < this->length; j++)
	if (this->get_cell(i, j)->value == 1)
		return this->get_cell(i, j);

	return nullptr;
}

grid::cell* grid::get_cell(int x, int y) {
	if (y < 0 || x < 0 || y >= this->length || x >= this->length)
		return nullptr;

	return this->cells.get() + (y * this->length + x);
}

int main(int argc, char** argv) {
	grid g;

	while (g.fill(cin)) {
		if (g.find_path())
			g.print();
		else
			cout << "No alibi" << endl;
	}

	return 0;
}