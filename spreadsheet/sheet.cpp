#include "sheet.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::Sheet()
    :table_size_({ 0,0 }) {}

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid cell position");
    }

    // Необходимо увеличить заданную область печати (размер таблицы)
    if (table_size_.cols <= pos.col || table_size_.rows <= pos.row) {

        // Если область печати меньше по высоте
        if (table_size_.rows <= pos.row) {
            size_t row_size = cell_pointers_.size();
            cell_pointers_.resize(pos.row + 1);
            for (size_t i = row_size; i < cell_pointers_.size(); ++i) {
                cell_pointers_[i].resize(table_size_.cols);
            }
            table_size_.rows = pos.row + 1;
        }

        // Если область печати меньше по ширине
        if (table_size_.cols <= pos.col) {
            for (std::vector<CellPtr>& vec : cell_pointers_) {
                vec.resize(pos.col + 1);
            }
            table_size_.cols = pos.col + 1;
        }

        cell_pointers_[pos.row][pos.col] = std::move(std::make_unique<Cell>(*this));
        cell_pointers_[pos.row][pos.col]->Set(text);
    }
    
    // Область печати (размер таблицы) шире позиции pos
    else {
        Cell* exist_cell = GetSimplyCell(pos);
        if (exist_cell) {
            exist_cell->Set(text);
        }
        else {

            cell_pointers_[pos.row][pos.col] = std::move(std::make_unique<Cell>(*this));
            cell_pointers_[pos.row][pos.col]->Set(text);
        }
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid cell position");
    }

    if (table_size_.cols <= pos.col || table_size_.rows <= pos.row) {
        return nullptr;
    }
    
    return cell_pointers_.at(pos.row).at(pos.col).get();
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid cell position");
    }

    if (table_size_.cols <= pos.col || table_size_.rows <= pos.row) {
        return nullptr;
    }
    
    return cell_pointers_[pos.row][pos.col].get();
}

const Cell* Sheet::GetSimplyCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid cell position");
    }

    if (table_size_.cols <= pos.col || table_size_.rows <= pos.row) {
        return nullptr;
    }

    return cell_pointers_.at(pos.row).at(pos.col).get();
}

Cell* Sheet::GetSimplyCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid cell position");
    }

    if (table_size_.cols <= pos.col || table_size_.rows <= pos.row) {
        return nullptr;
    }

    return cell_pointers_[pos.row][pos.col].get();
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid cell position");
    } 


    if (table_size_.cols > pos.col && table_size_.rows > pos.row) {
        if (cell_pointers_.at(pos.row).at(pos.col).get()) {
            cell_pointers_[pos.row][pos.col] = nullptr;
        }
    }

    if (table_size_.cols == pos.col + 1 && table_size_.rows == pos.row + 1) {
        Sheet::SetColSize();
        Sheet::SetRowSize();
    }
    else if (table_size_.cols == pos.col + 1) {
        Sheet::SetColSize();
    }
    else {
        Sheet::SetRowSize();
    }
}

Size Sheet::GetPrintableSize() const {
    return table_size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int y = 0; y < table_size_.rows; ++y) {
        bool first = false;
        for (int x = 0; x < table_size_.cols; ++x) {
            if (first) {
                output << "\t";
            }
            first = true;
            if (cell_pointers_.at(y).at(x).get()) {
                Cell::Value current_value = cell_pointers_[y][x].get()->GetValue();
                if (std::holds_alternative<FormulaError>(current_value)) {
                    output << std::get<FormulaError>(current_value);
                }
                else if (std::holds_alternative<double>(current_value)) {
                    output << std::get<double>(current_value);
                }
                else {
                    output << std::get<std::string>(current_value);
                }
            }
        }
        output << "\n";
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    for (int y = 0; y < table_size_.rows; ++y) {
        bool first = false;
        for (int x = 0; x < table_size_.cols; ++x) {
            if (first) {
                output << "\t";
            }
            first = true;
            if (cell_pointers_.at(y).at(x).get()) {
                output << cell_pointers_.at(y).at(x).get()->GetText();
            }
        }
        output << "\n";
    }
}

void Sheet::SetColSize() {
    int x = table_size_.cols - 1;
    table_size_.cols = 0;
    for (; x >= 0; --x) {
        for (int y = 0; y < table_size_.rows; ++y) {
            if (cell_pointers_.at(y).at(x).get()) {
                table_size_.cols = x + 1;
                return;
            }
        }
    }
}

void Sheet::SetRowSize() {
    int y = table_size_.rows - 1;
    table_size_.rows = 0;
    for (; y >= 0; --y) {
        for (int x = 0; x < table_size_.cols; ++x) {
            if (cell_pointers_.at(y).at(x).get()) {
                table_size_.rows = y + 1;
                return;
            }
        }
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
