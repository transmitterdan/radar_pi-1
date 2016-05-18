/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Navico BR24 Radar Plugin
 * Author:   David Register
 *           Dave Cowell
 *           Kees Verruijt
 *           Douwe Fokkema
 *           Sean D'Epagnier
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register              bdbcat@yahoo.com *
 *   Copyright (C) 2012-2013 by Dave Cowell                                *
 *   Copyright (C) 2012-2016 by Kees Verruijt         canboat@verruijt.net *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#ifndef _RADARDRAWVERTEX_H_
#define _RADARDRAWVERTEX_H_

#include "RadarDraw.h"

PLUGIN_BEGIN_NAMESPACE

#define BUFFER_SIZE (2000000)

class RadarDrawVertex : public RadarDraw {
 public:
  RadarDrawVertex(br24radar_pi* pi) {
    m_pi = pi;

    memset(m_vertices, 0, sizeof(m_vertices));
    m_count = 0;
    m_oom = false;

    // initialise polar_to_cart_y[arc + 1][radius] arrays
    for (int arc = 0; arc < LINES_PER_ROTATION + 1; arc++) {
      GLfloat sine = sinf((GLfloat)arc * PI * 2 / LINES_PER_ROTATION);
      GLfloat cosine = cosf((GLfloat)arc * PI * 2 / LINES_PER_ROTATION);
      for (int radius = 0; radius < RETURNS_PER_LINE + 1; radius++) {
        polar_to_cart_y[arc][radius] = (GLfloat)radius * sine;
        polar_to_cart_x[arc][radius] = (GLfloat)radius * cosine;
      }
    }
  }

  bool Init(int color_option);
  void DrawRadarImage(wxPoint center, double scale, double rotation);
  void ProcessRadarSpoke(SpokeBearing angle, UINT8* data, size_t len);

  ~RadarDrawVertex() {
    wxMutexLocker lock(m_mutex);

    for (size_t i = 0; i < LINES_PER_ROTATION; i++) {
      if (m_vertices[i].points) {
        free(m_vertices[i].points);
      }
    }
  }

 private:
  br24radar_pi* m_pi;

  static const int VERTEX_PER_TRIANGLE = 3;
  static const int VERTEX_PER_QUAD = 2 * VERTEX_PER_TRIANGLE;
  static const int MAX_BLOBS_PER_LINE = RETURNS_PER_LINE;

  struct VertexPoint {
    GLfloat x;
    GLfloat y;
    GLubyte red;
    GLubyte green;
    GLubyte blue;
    GLubyte alpha;
  };

  GLfloat polar_to_cart_x[LINES_PER_ROTATION + 1][RETURNS_PER_LINE + 1];
  GLfloat polar_to_cart_y[LINES_PER_ROTATION + 1][RETURNS_PER_LINE + 1];

  struct VertexLine {
    VertexPoint* points;
    time_t lastSeen;
    size_t count;
    size_t allocated;
  };

  wxMutex m_mutex;  // protects the following
  VertexLine m_vertices[LINES_PER_ROTATION];
  unsigned int m_count;
  bool m_oom;

  void SetBlob(VertexLine* line, int angle_begin, int angle_end, int r1, int r2, GLubyte red, GLubyte green, GLubyte blue,
               GLubyte alpha);
};

PLUGIN_END_NAMESPACE

#endif /* _RADARDRAWVERTEX_H_ */
