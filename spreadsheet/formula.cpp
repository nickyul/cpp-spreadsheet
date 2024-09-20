#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {

    struct ref_cell_hasher {
        size_t operator()(const Position& pos) const {
            std::hash<int> hasher;
            return hasher(pos.row) + hasher(pos.col) * 37;
        }
    };
    
    class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression) try
        : ast_(ParseFormulaAST(expression)) {}
    catch (...) {
        throw FormulaException("Can't make operand real");
    }

    Value Evaluate(const SheetInterface& sheet) const override {
        try {
            std::function<double(Position)> args = [&sheet](const Position pos) {
                if (!pos.IsValid()) {
                    throw FormulaError(FormulaError::Category::Ref);
                }

                const CellInterface* cell = sheet.GetCell(pos);

                if (cell) {

                    if (std::holds_alternative<double>(cell->GetValue())) {
                        return std::get<double>(cell->GetValue());

                    }
                    else if (std::holds_alternative<std::string>(cell->GetValue())) {
                        std::string value = std::get<std::string>(cell->GetValue());
                        double result = 0;
                        if (!value.empty()) {
                            std::istringstream input(value);
                            if (!(input >> result) || !input.eof()) {
                                throw FormulaError(FormulaError::Category::Value);
                            }
                        }
                        return result;
                    }
                    else {
                        throw FormulaError(std::get<FormulaError>(cell->GetValue()));
                    }

                }
                else {
                    return 0.0;
                }
            };
            return ast_.Execute(args);
        }

        catch (const FormulaError& error) {
            return error;
        }
    }

    std::string GetExpression() const override {
        std::ostringstream out;
        ast_.FormulaAST::PrintFormula(out);
        return out.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        std::vector<Position> result;
        std::unordered_set<Position, ref_cell_hasher> seen;

        for (const Position& pos : ast_.GetCells()) {
            if (seen.find(pos) == seen.end()) {
                result.push_back(pos);
                seen.insert(pos);
            }
        }

        return result;
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}
