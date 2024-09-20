#pragma once

#include "common.h"
#include "formula.h"
#include <set>
#include <unordered_set>
#include <optional>
#include <vector>

#include <stack>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;
    void InvalidateAllCache(bool first = false);



private:

    class Impl {
    public:
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const;

        virtual bool HasCache() const;
        virtual void InvalidateCache();

        virtual ~Impl() = default;
    };

    class EmptyImpl : public Impl {
    public:
        Value GetValue() const override;
        std::string GetText() const override;
    };

    class TextImpl : public Impl {
    public:
        TextImpl(std::string text);

        Value GetValue() const override;
        std::string GetText() const override;

    private:
        std::string value_;
    };

    class FormulaImpl : public Impl {
    public:
        FormulaImpl(std::string text, Sheet& sheet);

        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const;

        bool HasCache() const override;
        void InvalidateCache() override;

    private:
        mutable std::optional<FormulaInterface::Value> cache_;
        std::unique_ptr<FormulaInterface> formula_ptr_;
        Sheet& sheet_;
    };

    std::unique_ptr<Impl> impl_;
    Sheet& sheet_;

    std::set<Cell*> reffer_to_cells_;
    std::set<Cell*> reffered_by_cells_;

    bool HasCircularDependency(const std::vector<Position>& referenced_cells) const;
    void InvalidateCache();
    void RefreshCells();
};
