#pragma once

#include "cell.h"
#include "common.h"

#include <memory>
#include <functional>
#include <vector>

using CellPtr = std::unique_ptr<Cell>;

class Sheet : public SheetInterface {
public:
    explicit Sheet();

    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    const Cell* GetSimplyCell(Position pos) const;
    Cell* GetSimplyCell(Position pos);

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:

    void SetColSize();
    void SetRowSize();

    Size table_size_;
    std::vector<std::vector<CellPtr>> cell_pointers_;
};
