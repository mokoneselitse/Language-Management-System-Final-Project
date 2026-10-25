#include "LatePenalty.h"

// mark = max(0, raw - 10% of maxMark per day) for 1 to 3 days late,
// and 0 for more than 3 days late.
// Example (max 50, raw 40): 0 days -> 40, 1 -> 35, 2 -> 30, 3 -> 25, 4+ -> 0.
double StandardLatePenalty::apply(double rawMark, double maxMark, int daysLate) {
    if (daysLate <= 0) return rawMark;
    if (daysLate > maxDaysPenalised) return 0.0;
    double mark = rawMark - ratePerDay * maxMark * daysLate;
    if (mark < 0) mark = 0.0;
    return mark;
}

double NoLatePenalty::apply(double rawMark, double /*maxMark*/, int /*daysLate*/) {
    return rawMark;
}
