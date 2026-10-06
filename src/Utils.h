#ifndef UTILS_H
#define UTILS_H
#include <FSWindow.h>
using Vector = FS::Vector;
using Vector2 = FS::Vector2;
using Colour = FS::Colour;
using Colourf = FS::Colourf;

inline Vector normalize(const Vector vec) {
    return vec / length(vec);
}
#endif