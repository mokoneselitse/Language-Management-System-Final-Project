// LatePenalty.h - the late-penalty rule as a Strategy (FR18, C2).
// MarkingService only calls apply(); the 10%-per-day rule lives only in
// StandardLatePenalty, so it can be swapped without touching MarkingService.
#pragma once

class ILatePenaltyPolicy {
public:
    virtual ~ILatePenaltyPolicy() = default;
    virtual double apply(double rawMark, double maxMark, int daysLate) = 0;
};

class StandardLatePenalty : public ILatePenaltyPolicy {
public:
    double apply(double rawMark, double maxMark, int daysLate) override;

private:
    double ratePerDay = 0.10;
    int maxDaysPenalised = 3;
};

class NoLatePenalty : public ILatePenaltyPolicy {
public:
    double apply(double rawMark, double maxMark, int daysLate) override;
};
