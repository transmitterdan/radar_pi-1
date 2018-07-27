/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Radar Plugin
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

#include "RadarDrawVertex.h"
#include "RadarInfo.h"

PLUGIN_BEGIN_NAMESPACE

bool RadarDrawVertex::Init(size_t spokes, size_t spoke_len_max) {
  wxCriticalSectionLocker lock(m_exclusive);

  if (m_spokes != spokes) {
    Reset();
  }
  m_spokes = spokes;                // How many spokes form a circle
  m_spoke_len_max = spoke_len_max;  // How long each spoke is (max)

  if (!m_vertices) {
    m_vertices = (VertexLine*)calloc(sizeof(VertexLine), m_spokes);
  }
  if (!m_vertices) {
    if (!m_oom) {
      wxLogError(wxT("radar_pi: Out of memory"));
      m_oom = true;
    }
    return false;
  }

  return true;
}

void RadarDrawVertex::Reset() {
  if (m_vertices) {
    for (size_t i = 0; i < m_spokes; i++) {
      if (m_vertices[i].points) {
        free(m_vertices[i].points);
      }
    }
    free(m_vertices);
    m_vertices = 0;
  }
}

#define ADD_VERTEX_POINT(angle, radius, r, g, b, a)                         \
  {                                                                         \
    line->points[count].xy = m_ri->m_polar_lookup->GetPoint(angle, radius); \
    line->points[count].red = r;                                            \
    line->points[count].green = g;                                          \
    line->points[count].blue = b;                                           \
    line->points[count].alpha = a;                                          \
    count++;                                                                \
  }

void RadarDrawVertex::SetBlob(VertexLine* line, int angle_begin, int angle_end, int r1, int r2, GLubyte red, GLubyte green,
                              GLubyte blue, GLubyte alpha) {
  if (r2 == 0) {
    return;
  }
  int arc1 = angle_begin % m_spokes;
  int arc2 = angle_end % m_spokes;
  size_t count = line->count;

  if (line->count + VERTEX_PER_QUAD > line->allocated) {
    const size_t extra = 8 * VERTEX_PER_QUAD;
    line->points = (VertexPoint*)realloc(line->points, (line->allocated + extra) * sizeof(VertexPoint));
    line->allocated += extra;
    m_count += extra;
  }

  if (!line->points) {
    if (!m_oom) {
      wxLogError(wxT("radar_pi: Out of memory"));
      m_oom = true;
    }
    return;
  }

  // First triangle
  ADD_VERTEX_POINT(arc1, r1, red, green, blue, alpha);
  ADD_VERTEX_POINT(arc1, r2, red, green, blue, alpha);
  ADD_VERTEX_POINT(arc2, r1, red, green, blue, alpha);

  // Second triangle

  ADD_VERTEX_POINT(arc2, r1, red, green, blue, alpha);
  ADD_VERTEX_POINT(arc1, r2, red, green, blue, alpha);
  ADD_VERTEX_POINT(arc2, r2, red, green, blue, alpha);

  line->count = count;
}

void RadarDrawVertex::ProcessRadarSpoke(int transparency, SpokeBearing angle, uint8_t* data, size_t len, GeoPosition spoke_pos) {
  wxColour colour;
  GLubyte alpha = 255 * (MAX_OVERLAY_TRANSPARENCY - transparency) / MAX_OVERLAY_TRANSPARENCY;
  BlobColour previous_colour = BLOB_NONE;
  GLubyte strength = 0;
  time_t now = time(0);

  wxCriticalSectionLocker lock(m_exclusive);

  int r_begin = 0;
  int r_end = 0;

  if (angle < 0 || angle >= (int)m_spokes || len > m_spoke_len_max || !m_vertices) {
    return;
  }

  VertexLine* line = &m_vertices[angle];

  if (!line->points) {
    static size_t INITIAL_ALLOCATION = 600;  // Empirically found to be enough for a complicated picture
    line->allocated = INITIAL_ALLOCATION * VERTEX_PER_QUAD;
    m_count += INITIAL_ALLOCATION * VERTEX_PER_QUAD;
    line->points = (VertexPoint*)malloc(line->allocated * sizeof(VertexPoint));
    if (!line->points) {
      if (!m_oom) {
        wxLogError(wxT("radar_pi: Out of memory"));
        m_oom = true;
      }
      line->allocated = 0;
      line->count = 0;
      return;
    }
  }
  line->count = 0;
  line->timeout = now + m_ri->m_pi->m_settings.max_age;
  line->spoke_pos = spoke_pos;

  for (size_t radius = 0; radius < len; radius++) {
    strength = data[radius];
    BlobColour actual_colour = m_ri->m_colour_map[strength];

    if (actual_colour == previous_colour) {
      // continue with same color, just register it
      r_end++;
    } else if (previous_colour == BLOB_NONE && actual_colour != BLOB_NONE) {
      // blob starts, no display, just register
      r_begin = radius;
      r_end = r_begin + 1;
      previous_colour = actual_colour;  // new color
    } else if (previous_colour != BLOB_NONE && (previous_colour != actual_colour)) {
      colour = m_ri->m_colour_map_rgb[previous_colour];

      SetBlob(line, angle, angle + 1, r_begin, r_end, colour.Red(), colour.Green(), colour.Blue(), alpha);

      previous_colour = actual_colour;
      if (actual_colour != BLOB_NONE) {  // change of color, start new blob
        r_begin = radius;
        r_end = r_begin + 1;
      }
    }
  }

  if (previous_colour != BLOB_NONE) {  // Draw final blob
    colour = m_ri->m_colour_map_rgb[previous_colour];

    SetBlob(line, angle, angle + 1, r_begin, r_end, colour.Red(), colour.Green(), colour.Blue(), alpha);
  }
}

void RadarDrawVertex::DrawRadarImage(double radar_scale, double panel_rotate) {
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  time_t now = time(0);
  {
    wxCriticalSectionLocker lock(m_exclusive);
    wxPoint boat_center;

    for (size_t i = 0; i < m_spokes; i++) {
      VertexLine* line = &m_vertices[i];
      if (!line->count || TIMED_OUT(now, line->timeout)) {
        continue;
      }

      if (line->spoke_pos.lat != m_prev_pos.lat || line->spoke_pos.lon != m_prev_pos.lon) {
        m_prev_pos = line->spoke_pos;
        GetCanvasPixLL(m_ri->m_pi->m_vp, &boat_center, line->spoke_pos.lat, line->spoke_pos.lon);
        // move display to the location where the spoke was recorded
        glPushMatrix();
        glTranslated(boat_center.x, boat_center.y, 0);
        glRotated(panel_rotate, 0.0, 0.0, 1.0);
        glScaled(radar_scale, radar_scale, 1.);
        glVertexPointer(2, GL_FLOAT, sizeof(VertexPoint), &line->points[0].xy);
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(VertexPoint), &line->points[0].red);
        glDrawArrays(GL_TRIANGLES, 0, line->count);
        glPopMatrix();
      }
      else {
        glVertexPointer(2, GL_FLOAT, sizeof(VertexPoint), &line->points[0].xy);
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(VertexPoint), &line->points[0].red);
        glDrawArrays(GL_TRIANGLES, 0, line->count);
      }
    }
  }
  glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
  glDisableClientState(GL_COLOR_ARRAY);
}

PLUGIN_END_NAMESPACE
