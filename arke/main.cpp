#include <iostream>
#include <iomanip>
#include <memory>
#include <chrono>

using namespace std;

struct cell {
	int x;
	int y;
	int value;
	int neighbor_count;
	cell* neighbors[4];
};

cell* cells = nullptr;
int length = 0;
chrono::steady_clock::time_point start_calc;
chrono::steady_clock::time_point end_calc;

cell* get_cell(int x, int y) {
	if (y < 0 || x < 0 || y >= length || x >= length)
		return nullptr;

	return cells + (y * length + x);
}

void print(ostream& destination) {
	destination << endl;
	destination << chrono::duration_cast<chrono::milliseconds>(end_calc - start_calc).count() << endl;

	for (int i = 0; i < length; i++) {
		for (int j = 0; j < length; j++) {
			destination << setw(2) << get_cell(j, i)->value << " ";
		}
		destination << endl;
	}

	destination << endl;
}

bool fill(istream& source) {
	source >> length;

	if (length < 1)
		return false;
	
	if (cells)
		delete[] cells;

	cells = new cell[length * length];

	for (int i = 0; i < length; i++) {
		for (int j = 0; j < length; j++) {
			auto current = get_cell(i, j);
			current->x = i;
			current->y = j;
			current->value = 0;
			current->neighbor_count = 0;

			auto current_neighbor = get_cell(i + 1, j);
			if (current_neighbor)
				current->neighbors[current->neighbor_count++] = current_neighbor;

			current_neighbor = get_cell(i - 1, j);
			if (current_neighbor)
				current->neighbors[current->neighbor_count++] = current_neighbor;

			current_neighbor = get_cell(i, j + 1);
			if (current_neighbor)
				current->neighbors[current->neighbor_count++] = current_neighbor;

			current_neighbor = get_cell(i, j - 1);
			if (current_neighbor)
				current->neighbors[current->neighbor_count++] = current_neighbor;
		}
	}

	for (int i = 0; i < length; i++)
		source >> get_cell(i, 0)->value;

	for (int i = 1; i < length - 1; i++)
		source >> get_cell(length - 1, i)->value;

	for (int i = length - 1; i >= 0; i--)
		source >> get_cell(i, length - 1)->value;

	for (int i = length - 2; i > 0; i--)
		source >> get_cell(0, i)->value;

	return true;
}

cell* find_start() {
	for (int i = 0; i < length; i++)
		for (int j = 0; j < length; j++)
			if (get_cell(i, j)->value == 1)
				return get_cell(i, j);

	return nullptr;
}

bool find_path(cell* current_location = nullptr, cell* last_location = nullptr) {
	if (current_location->value == length * length) {
		end_calc = chrono::steady_clock::now();
		return true;
	}

	for (int i = 0; i < current_location->neighbor_count; i++)
		if (current_location->neighbors[i]->value == current_location->value + 1)
			return find_path(current_location->neighbors[i]);

	for (int i = 0; i < current_location->neighbor_count; i++) {
		auto neighbor = current_location->neighbors[i];

		if (neighbor != last_location && neighbor->value == 0) {
			neighbor->value = current_location->value + 1;

			if (find_path(neighbor, current_location))
				return true;

			neighbor->value = 0;
		}
	}

	return false;
}

int main(int argc, char** argv) {
	while (fill(cin)) {
		start_calc = chrono::steady_clock::now();
		if (find_path(find_start()))
			print(cout);
		else
			cout << "No alibi" << endl;
	}

	return 0;
}
