#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


Cell::Cell(Sheet& sheet)
	:impl_(std::make_unique<EmptyImpl>()), sheet_(sheet) {}

Cell::~Cell() = default;

// Задаёт содержимое ячейки. Если текст начинается со знака "=", то он
// интерпретируется как формула. Уточнения по записи формулы:
// * Если текст содержит только символ "=" и больше ничего, то он не считается
// формулой
// * Если текст начинается с символа "'" (апостроф), то при выводе значения
// ячейки методом GetValue() он опускается. Можно использовать, если нужно
// начать текст со знака "=", но чтобы он не интерпретировался как формула.
void Cell::Set(std::string text) {
	std::unique_ptr<Impl> help;

	if (text.empty()) {
		help = std::make_unique<EmptyImpl>();
	}
	else if (text.size() > 1 && text.at(0) == FORMULA_SIGN) {
		help = std::make_unique<FormulaImpl>(text, sheet_);
	}
	else {
		help = std::make_unique<TextImpl>(text);
	}

	if (HasCircularDependency(help->GetReferencedCells())) {
		throw CircularDependencyException("Here is circular dependency");
	}
	impl_ = std::move(help);

	RefreshCells();
	InvalidateAllCache(true);
}

void Cell::Clear() {
	Set("");
}

// Возвращает видимое значение ячейки.
// В случае текстовой ячейки это её текст (без экранирующих символов). В
// случае формулы - числовое значение формулы или сообщение об ошибке.
Cell::Value Cell::GetValue() const {
	return impl_->GetValue();
}

// Возвращает внутренний текст ячейки, как если бы мы начали её
// редактирование. В случае текстовой ячейки это её текст (возможно,
// содержащий экранирующие символы). В случае формулы - её выражение.
std::string Cell::GetText() const {
	return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
	return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const {
	return {};
}

void Cell::InvalidateAllCache(bool first) {
	if (impl_->HasCache() || first) {
		impl_->InvalidateCache();

		for (Cell* cell : reffered_by_cells_) {
			cell->InvalidateAllCache();
		}
	}
}

struct CellHasher {
	size_t operator()(const Cell* cell) const {
		std::hash<const void*> hasher;
		return hasher(cell);
	}
};
					
bool Cell::HasCircularDependency(const std::vector<Position>& referenced_cells) const {
	if (referenced_cells.empty()) {
		return false;
	}

	std::unordered_set<const Cell*, CellHasher> referenced_cells_set;
	std::unordered_set<const Cell*, CellHasher> visited_cells;
	std::vector<const Cell*> cells_to_check;

	for (const Position& pos : referenced_cells) {
		referenced_cells_set.insert(sheet_.GetSimplyCell(pos));
	}

	if (referenced_cells_set.find(this) != referenced_cells_set.end()) {
		return true;
	}

	cells_to_check.push_back(this);

	while (!cells_to_check.empty()) {
		const Cell* current_cell = cells_to_check.back();
		cells_to_check.pop_back();
		visited_cells.insert(current_cell);

		if (referenced_cells_set.find(current_cell) == referenced_cells_set.end()) {
			// Если ячейка не ссылается на саму себя, то проверим ее зависимости
			for (const Cell* dependent_cell : current_cell->reffered_by_cells_) {
				if (visited_cells.find(dependent_cell) == visited_cells.end()) {
					cells_to_check.push_back(dependent_cell);
				}
			}
		}
		else {
			// Ячейка ссылается на саму себя
			return true;
		}
	}

	return false;
}

void Cell::InvalidateCache() {}

void Cell::RefreshCells() {
	for (Cell* cell : reffer_to_cells_) {
		cell->reffered_by_cells_.erase(this);
	}
	reffer_to_cells_.clear();
	for (const Position& pos : impl_->GetReferencedCells()) {

		Cell* curr_iter = sheet_.GetSimplyCell(pos);
		if (!curr_iter) {

			sheet_.SetCell(pos, "");
			curr_iter = sheet_.GetSimplyCell(pos);
		}
		reffer_to_cells_.insert(curr_iter);
		curr_iter->reffered_by_cells_.insert(this);
	}
}

Cell::Value Cell::EmptyImpl::GetValue() const {
	return std::string();
}

std::string Cell::EmptyImpl::GetText() const {
	return std::string();
}

Cell::TextImpl::TextImpl(std::string text)
	:value_(std::move(text)) {}

Cell::Value Cell::TextImpl::GetValue() const {
	if (value_.at(0) == ESCAPE_SIGN) {
		return value_.substr(1);
	}
	else {
		return value_;
	}
}

std::string Cell::TextImpl::GetText() const {
	return value_;
}

Cell::FormulaImpl::FormulaImpl(std::string text, Sheet& sheet)
	:cache_(std::nullopt), formula_ptr_(std::move(ParseFormula(text.substr(1)))), sheet_(sheet)  {}

Cell::Value Cell::FormulaImpl::GetValue() const {
	if (!cache_) { 
		cache_ = formula_ptr_->Evaluate(sheet_); 
	}

	if (std::holds_alternative<double>(cache_.value())) {
		return std::get<double>(cache_.value());
	}
	else {
		return std::get<FormulaError>(cache_.value());
	}
}

std::string Cell::FormulaImpl::GetText() const {
	return FORMULA_SIGN + formula_ptr_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
	return formula_ptr_->GetReferencedCells();
}

bool Cell::FormulaImpl::HasCache() const {
	return cache_.has_value();
}

void Cell::FormulaImpl::InvalidateCache() {
	cache_.reset();
}

std::vector<Position> Cell::Impl::GetReferencedCells() const {
	return {};
}

bool Cell::Impl::HasCache() const {
	return true;
}

void Cell::Impl::InvalidateCache() {
	
}
