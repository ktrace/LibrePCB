/*
 * LibrePCB - Professional EDA for everyone!
 * Copyright (C) 2013 LibrePCB Developers, see AUTHORS.md for contributors.
 * https://librepcb.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*******************************************************************************
 *  Includes
 ******************************************************************************/
#include "utils/toolbox.h"

#include <librepcb/rust-core/ffi.h>

#include <QtCore>

/*******************************************************************************
 *  Namespace
 ******************************************************************************/
namespace librepcb {

/*******************************************************************************
 *  Static Methods
 ******************************************************************************/

bool Toolbox::isTextUpsideDown(const Angle& rotation) noexcept {
  const Angle mapped180 = rotation.mappedTo180deg();
  return ((mapped180 <= -Angle::deg90()) || (mapped180 > Angle::deg90()));
}

QPainterPath Toolbox::shapeFromPath(const QPainterPath& path, const QPen& pen,
                                    const QBrush& brush,
                                    const UnsignedLength& minWidth) noexcept {
  // http://code.qt.io/cgit/qt/qtbase.git/tree/src/widgets/graphicsview/qgraphicsitem.cpp
  // Function: qt_graphicsItem_shapeFromPath()

  if ((path.isEmpty()) || (pen.style() == Qt::NoPen) ||
      (pen.brush().style() == Qt::NoBrush)) {
    return path;
  } else {
    QPainterPathStroker ps;
    ps.setCapStyle(pen.capStyle());
    ps.setWidth(qMax(qMax(pen.widthF(), qreal(0.00000001)), minWidth->toPx()));
    ps.setJoinStyle(pen.joinStyle());
    ps.setMiterLimit(pen.miterLimit());
    QPainterPath p = ps.createStroke(path);
    if (brush.style() != Qt::NoBrush) {
      p.addPath(path);
    }
    return p;
  }
}

std::optional<Length> Toolbox::arcRadius(const Point& p1, const Point& p2,
                                         const Angle& angle) noexcept {
  const Angle angleMapped = angle.mappedTo180deg();
  if ((angleMapped == 0) || (p1 == p2)) {
    return std::nullopt;  // Given input is a straight line.
  }

  const qreal x1 = p1.getX().toMm();
  const qreal y1 = p1.getY().toMm();
  const qreal x2 = p2.getX().toMm();
  const qreal y2 = p2.getY().toMm();
  const qreal d = std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
  const qreal r = d / (2 * std::sin(angleMapped.toRad() / 2));
  if (Length::isValidMm(r)) {
    return Length::fromMm(r);
  } else {
    return std::nullopt;  // Too large radius.
  }
}

std::optional<Point> Toolbox::arcCenter(const Point& p1, const Point& p2,
                                        const Angle& angle) noexcept {
  const Angle angleMapped = angle.mappedTo180deg();
  if ((angleMapped == 0) || (p1 == p2)) {
    return std::nullopt;  // Given input is a straight line.
  }

  // http://math.stackexchange.com/questions/27535/how-to-find-center-of-an-arc-given-start-point-end-point-radius-and-arc-direc
  const qreal x0 = p1.getX().toMm();
  const qreal y0 = p1.getY().toMm();
  const qreal x1 = p2.getX().toMm();
  const qreal y1 = p2.getY().toMm();
  const qreal angleSgn = (angleMapped >= 0) ? 1 : -1;
  const qreal d = std::sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
  const qreal r = d / (2 * std::sin(angleMapped.toRad() / 2));
  // Note: std::max() fixes https://github.com/LibrePCB/LibrePCB/issues/974
  const qreal h = std::sqrt(std::max(r * r - d * d / 4.0, qreal(0)));
  const qreal u = (x1 - x0) / d;
  const qreal v = (y1 - y0) / d;
  const qreal a = ((x0 + x1) / 2) - h * v * angleSgn;
  const qreal b = ((y0 + y1) / 2) + h * u * angleSgn;
  if (Length::isValidMm(a) && Length::isValidMm(b)) {
    return Point::fromMm(a, b);
  } else {
    return std::nullopt;  // Center too far away, consider it as a straight
                          // line.
  }
}

Angle Toolbox::arcAngle(const Point& p1, const Point& p2,
                        const Point& center) noexcept {
  Point delta1 = p1 - center;
  Point delta2 = p2 - center;
  if (delta1.isOrigin() || delta2.isOrigin()) {
    return Angle::deg0();
  }
  qreal angle1 = std::atan2(delta1.getY().toMm(), delta1.getX().toMm());
  qreal angle2 = std::atan2(delta2.getY().toMm(), delta2.getX().toMm());
  return Angle::fromRad(angle2 - angle1).mapTo0_360deg();
}

Angle Toolbox::arcAngleFrom3Points(const Point& start, const Point& mid,
                                   const Point& end) noexcept {
  const UnsignedLength h = shortestDistanceBetweenPointAndLine(mid, start, end);
  const UnsignedLength c = (end - start).getLength();
  const qreal phi = 4 * std::atan((2 * h->toMm()) / c->toMm());
  const Angle angleMid = angleBetweenPoints(start, mid);
  const Angle angleEnd = angleBetweenPoints(start, end);
  const Angle angleDelta = angleMid - angleEnd;
  if (angleDelta.mappedTo180deg() < 0) {
    return Angle::fromRad(phi);
  } else {
    return -Angle::fromRad(phi);
  }
}

Angle Toolbox::angleBetweenPoints(const Point& p1, const Point& p2) noexcept {
  const Point delta = p2 - p1;
  if (delta.isOrigin()) {
    return Angle::deg0();
  }
  return Angle::fromRad(std::atan2(delta.getY().toMm(), delta.getX().toMm()))
      .mapTo0_360deg();
}

Point Toolbox::nearestPointOnLine(const Point& p, const Point& l1,
                                  const Point& l2) noexcept {
  Point a = l2 - l1;
  Point b = p - l1;
  Point c = p - l2;
  qreal d = ((b.getX().toMm() * a.getX().toMm()) +
             (b.getY().toMm() * a.getY().toMm()));
  qreal e = ((a.getX().toMm() * a.getX().toMm()) +
             (a.getY().toMm() * a.getY().toMm()));
  if (a.isOrigin() || b.isOrigin() || (d <= 0.0)) {
    return l1;
  } else if (c.isOrigin() || (e <= d)) {
    return l2;
  } else {
    Q_ASSERT(e > 0.0);
    return l1 + Point::fromMm(a.getX().toMm() * d / e, a.getY().toMm() * d / e);
  }
}

UnsignedLength Toolbox::shortestDistanceBetweenPointAndLine(
    const Point& p, const Point& l1, const Point& l2, Point* nearest) noexcept {
  Point np = nearestPointOnLine(p, l1, l2);
  if (nearest) {
    *nearest = np;
  }
  return (p - np).getLength();
}

QString Toolbox::incrementNumberInString(QString string) noexcept {
  rs::ffi_increment_number_in_string(&string);
  return string;
}

QStringList Toolbox::expandRangesInString(const QString& string) noexcept {
  // Do NOT accept '+' and '-', they are considered as strings, not numbers!
  // For example in the range connector signals range "X-1..10" you expect
  // numbers starting from 1, not -1.
  QString number = "\\d+";
  QString character = "[a-zA-Z]";
  QString separator = "\\.\\.";
  QString numberRange =
      QString("(?<num_start>%1)%2(?<num_end>%1)").arg(number, separator);
  QString characterRange =
      QString("(?<char_start>%1)%2(?<char_end>%1)").arg(character, separator);
  QString pattern =
      QString("(?<num>%1)|(?<char>%2)").arg(numberRange, characterRange);
  QRegularExpression re(pattern);
  QRegularExpressionMatchIterator it = re.globalMatch(string);
  QVector<std::tuple<int, int, QStringList>> replacements;
  while (it.hasNext()) {
    QRegularExpressionMatch match = it.next();
    QStringList matchReplacements;
    if (!match.captured("num").isEmpty()) {
      bool okStart = false, okEnd = false;
      int start = match.captured("num_start").toInt(&okStart);
      int end = match.captured("num_end").toInt(&okEnd);
      if (okStart && okEnd) {
        bool invert = start > end;
        if (invert) std::swap(start, end);
        for (int i = start; i <= end; ++i) {
          if (invert) {
            matchReplacements.prepend(QString::number(i));
          } else {
            matchReplacements.append(QString::number(i));
          }
        }
      }
    } else if (!match.captured("char").isEmpty()) {
      QString startStr = match.captured("char_start");
      QString endStr = match.captured("char_end");
      if ((startStr.length()) == 1 && (endStr.length() == 1)) {
        int start = startStr[0].unicode();
        int end = endStr[0].unicode();
        bool invert = start > end;
        if (invert) std::swap(start, end);
        if (((start >= 'a') && (end <= 'z')) ||
            ((start >= 'A') && (end <= 'Z'))) {
          for (int i = start; i <= end; ++i) {
            if (invert) {
              matchReplacements.prepend(QChar::fromLatin1(i));
            } else {
              matchReplacements.append(QChar::fromLatin1(i));
            }
          }
        }
      }
    }
    // allow max. 4 replacements to avoid huge results
    if (!matchReplacements.isEmpty() && (replacements.count() < 4)) {
      replacements.append(std::make_tuple(
          match.capturedStart(), match.capturedLength(), matchReplacements));
    }
  }
  return expandRangesInString(string, replacements);
}

QString Toolbox::cleanUserInputString(const QString& input,
                                      const QRegularExpression& removeRegex,
                                      bool trim, bool toLower, bool toUpper,
                                      const QString& spaceReplacement,
                                      int maxLength) noexcept {
  // perform compatibility decomposition (NFKD)
  QString ret = input.normalized(QString::NormalizationForm_KD);
  // change case of all characters
  if (toLower) ret = ret.toLower();
  if (toUpper) ret = ret.toUpper();
  // remove leading and trailing spaces
  if (trim) ret = ret.trimmed();
  // replace remaining spaces with replacement
  ret.replace(" ", spaceReplacement);
  // remove all invalid characters
  ret.remove(removeRegex);
  // truncate to maximum allowed length
  if (maxLength >= 0) ret.truncate(maxLength);
  // if there are leading or trailing spaces, remove them again ;)
  if (trim) ret = ret.trimmed();
  return ret;
}

QString Toolbox::prettyPrintLocale(const QString& code) noexcept {
  QLocale locale(code);
  QString str = locale.nativeLanguageName();
  if (str.isEmpty()) {
    str = code;  // fallback if language code is not recognized
  }
  if (!locale.nativeCountryName().isEmpty()) {
    str += " (" % locale.nativeCountryName() % ")";
  }
  return str;
}

/*******************************************************************************
 *  Private Methods
 ******************************************************************************/

QStringList Toolbox::expandRangesInString(
    const QString& input,
    const QVector<std::tuple<int, int, QStringList>>& replacements) noexcept {
  if (replacements.isEmpty()) {
    return QStringList{input};
  } else {
    QStringList result;
    int pos = std::get<0>(replacements.first());
    int len = std::get<1>(replacements.first());
    foreach (const QString& replacement, std::get<2>(replacements.first())) {
      foreach (QString str, expandRangesInString(input, replacements.mid(1))) {
        result.append(str.replace(pos, len, replacement));
      }
    }
    return result;
  }
}

/*******************************************************************************
 *  End of File
 ******************************************************************************/

}  // namespace librepcb
