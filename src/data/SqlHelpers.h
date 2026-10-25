// SqlHelpers.h - small helpers for nullable columns and timestamps,
// shared by the repository .cpp files.
#pragma once

#include <chrono>
#include <optional>
#include <string>

#include "Statement.h"
#include "Types.h"

inline long long nowUnix() { return toUnixSeconds(std::chrono::system_clock::now()); }

// Bind an optional value: empty -> NULL.
inline void bindOptional(Statement& st, int index, std::optional<int> value) {
    if (value) st.bind(index, *value); else st.bindNull(index);
}
inline void bindOptional(Statement& st, int index, std::optional<double> value) {
    if (value) st.bind(index, *value); else st.bindNull(index);
}
inline void bindOptional(Statement& st, int index, std::optional<Timestamp> value) {
    if (value) st.bind(index, toUnixSeconds(*value)); else st.bindNull(index);
}

// Read a nullable column: NULL -> empty optional.
inline std::optional<int> readOptionalInt(Statement& st, int col) {
    if (st.isNull(col)) return std::nullopt;
    return st.columnInt(col);
}
inline std::optional<double> readOptionalDouble(Statement& st, int col) {
    if (st.isNull(col)) return std::nullopt;
    return st.columnDouble(col);
}
inline std::optional<Timestamp> readOptionalTime(Statement& st, int col) {
    if (st.isNull(col)) return std::nullopt;
    return fromUnixSeconds(st.columnInt64(col));
}
inline Timestamp readTime(Statement& st, int col) { return fromUnixSeconds(st.columnInt64(col)); }
