#include "common.h"

#include <algorithm>
#include <cctype>
#include <sstream>

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

const Position Position::NONE = { -1, -1 };

// Реализуйте методы:
bool Position::operator==(const Position rhs) const {
	return col == rhs.col && row == rhs.row;
}

bool Position::operator<(const Position rhs) const {
	return row < rhs.row && col < rhs.col;
}

bool Position::IsValid() const {
	return row >= 0
		&& row < MAX_ROWS
		&& col >= 0
		&& col < MAX_COLS;
}

std::string Position::ToString() const {
	if (!IsValid()) {
		return std::string{};
	}

	std::string result;
	int help = col;

	while (help >= 0) {
		result.insert(result.begin(), 'A' + help % LETTERS);
		help = help / LETTERS - 1;
	}

	result += std::to_string(row + 1);
	return result;
}

Position Position::FromString(std::string_view str) {
	std::string_view::iterator digits_it = std::find_if(str.begin(), str.end(), [](char ch) {
		return !std::isalpha(ch);
		});

	std::string_view letters = str.substr(0, digits_it - str.begin());
	std::string_view digits = str.substr(digits_it - str.begin());

	if (letters.empty() || digits.empty()) {
		return Position::NONE;
	}

	for (char ch : letters) {
		if (!std::isupper(ch)) {
			return Position::NONE;
		}
	}

	for (char dig : digits) {
		if (!std::isdigit(dig)) {
			return Position::NONE;
		}
	}

	int col = 0;

	if (digits.size() > 5) {
		return Position::NONE;
	}

	int row = std::stoi(std::string(digits)) - 1;

	for (char ch : letters) {
		col *= LETTERS;
		col += ch - 'A' + 1;
	}
	--col;

	if (!Position{ row,col }.IsValid()) {
		return Position::NONE;
	}

	return { row, col };
}

bool Size::operator==(Size rhs) const {
	return rows == rhs.rows && cols == rhs.cols;
}
