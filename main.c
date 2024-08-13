#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

#define WIDTH 1200
#define HEIGHT 1200
#define BYTE_SIZE 8
#define RADIUS 15
#define SEED_AMOUNT 12
static uint32_t image[HEIGHT][WIDTH];
#define COLOR_RED 0xFFFF0000
#define COLOR_GREEN 0xFF00FF00
#define COLOR_BLUE 0xFF0000FF
#define COLOR_YELLOW 0xFFFFFF00
#define COLOR_CYAN 0xFF00FFFF
#define COLOR_MAGENTA 0xFFFF00FF
#define COLOR_ORANGE 0xFFFFA500
#define COLOR_PURPLE 0xFF800080
#define COLOR_TEAL 0xFF008080
#define COLOR_LIME 0xFF00FF00
#define COLOR_PINK 0xFFFFC0CB
#define COLOR_BROWN 0xFFA52A2A
#define COLOR_GRAY 0xFF808080
#define COLOR_BLACK 0xFF000000
#define COLOR_WHITE 0xFFFFFFFF
#define COLORS_AMOUNT 15

// Store all colors in a uint32_t array
uint32_t colors[15] = {COLOR_RED,  COLOR_GREEN,   COLOR_BLUE,   COLOR_YELLOW,
                       COLOR_CYAN, COLOR_MAGENTA, COLOR_ORANGE, COLOR_PURPLE,
                       COLOR_TEAL, COLOR_LIME,    COLOR_PINK,   COLOR_BROWN,
                       COLOR_GRAY, COLOR_BLACK,   COLOR_WHITE};
typedef uint32_t Color32;

typedef struct SPoint {
  uint32_t x, y;
} Point;
static Point seeds[SEED_AMOUNT];

void fill_image(Color32 color) {
  for (size_t y = 0; y < HEIGHT; ++y) {
    for (size_t x = 0; x < WIDTH; ++x) {
      image[y][x] = color;
    }
  }
}

void save_as_ppm(const char *file_name) {
  FILE *file = fopen(file_name, "wb");
  assert(file != NULL);
  fprintf(file, "P6\n%d %d 255\n", WIDTH, HEIGHT);

  for (size_t y = 0; y < HEIGHT; ++y) {
    for (size_t x = 0; x < WIDTH; ++x) {
      uint8_t bytes[3];
      // 0xAABBGGRR
      bytes[0] = (image[y][x] & 0x00FF) >> 0 * BYTE_SIZE;
      bytes[1] = (image[y][x] & 0xFF00) >> BYTE_SIZE * 1;
      bytes[2] = (image[y][x] & 0xFF0000) >> BYTE_SIZE * 2;
      fwrite(bytes, sizeof(bytes), 1, file);
      assert(!ferror(file));
    }
  }
  int closed = fclose(file);
  assert(closed == 0);
}

void generate_seeds() {
  for (size_t i = 0; i < SEED_AMOUNT; ++i) {
    seeds[i].x = rand() % WIDTH;
    seeds[i].y = rand() % HEIGHT;
  }
}

void draw_circle(Point point) {

  uint32_t x0 = point.x - RADIUS, x1 = point.x + RADIUS, y0 = point.y - RADIUS,
           y1 = point.y + RADIUS;
  for (size_t y = y0; y < y1; ++y) {
    for (size_t x = x0; x < x1; ++x) {
      uint32_t dx = point.x - x, dy = point.y - y;
      if (x <= WIDTH && y <= HEIGHT) {
        if (dx * dx + dy * dy < RADIUS * RADIUS)
          image[y][x] = COLOR_WHITE;
      }
    }
  }
}

void fill_seeds() {

  for (size_t i = 0; i < SEED_AMOUNT; ++i) {
    draw_circle(seeds[i]);
  }
}

void draw_voronoi() {

  Color32 color;
  uint32_t minimal_dist = UINT32_MAX, dist;
  for (size_t y = 0; y < HEIGHT; ++y) {
    for (size_t x = 0; x < WIDTH; ++x) {
      minimal_dist = UINT32_MAX;
      for (size_t i = 0; i < SEED_AMOUNT; ++i) {
        Point point = seeds[i];
        uint32_t vert_dist = point.y - y, horiz_dist = point.x - x;
        dist = vert_dist * vert_dist + horiz_dist * horiz_dist;
        if (dist < minimal_dist) {
          color = colors[i % COLORS_AMOUNT];
          minimal_dist = dist;
        }
      }
      image[y][x] = color;
    }
  }
}

void draw_voronoi_taxicab() {

  Color32 color;
  uint32_t minimal_dist = UINT32_MAX, dist;
  for (size_t y = 0; y < HEIGHT; ++y) {
    for (size_t x = 0; x < WIDTH; ++x) {
      minimal_dist = UINT32_MAX;
      for (size_t i = 0; i < SEED_AMOUNT; ++i) {
        Point point = seeds[i];
        dist = abs(point.x - x) + abs(point.y - y);
        if (dist < minimal_dist) {
          color = colors[i % COLORS_AMOUNT];
          minimal_dist = dist;
        }
      }
      image[y][x] = color;
    }
  }
}

void draw_voronoi_chebyshev() {

  Color32 color;
  uint32_t minimal_dist = UINT32_MAX, dist;
  for (size_t y = 0; y < HEIGHT; ++y) {
    for (size_t x = 0; x < WIDTH; ++x) {
      minimal_dist = UINT32_MAX;
      for (size_t i = 0; i < SEED_AMOUNT; ++i) {
        Point point = seeds[i];
        dist = abs(point.x - x);
        if (abs(point.y - y) > dist)
          dist = abs(point.y - y);
        if (dist < minimal_dist) {
          color = colors[i % COLORS_AMOUNT];
          minimal_dist = dist;
        }
      }
      image[y][x] = color;
    }
  }
}
uint32_t cube_root_approx(uint32_t n) {
  uint32_t x = n;
  uint32_t y = (x + 2) / 3;
  while (y < x) {
    x = y;
    y = (x + n / (x * x)) / 3;
  }
  return x;
}
uint32_t minkovski_norm(uint32_t x1, uint32_t x2, uint32_t y1, uint32_t y2) {
  uint32_t dx = abs(x1 - x2), dy = abs(y1 - y2);
  uint32_t dist = (uint32_t)cbrt((double)(dx * dx * dx + dy * dy * dy));
  return dist;
}

void draw_voronoi_minkovski() {

  Color32 color;
  uint32_t minimal_dist = UINT32_MAX, dist;
  for (size_t y = 0; y < HEIGHT; ++y) {
    for (size_t x = 0; x < WIDTH; ++x) {
      minimal_dist = UINT32_MAX;
      for (size_t i = 0; i < SEED_AMOUNT; ++i) {
        Point point = seeds[i];
        dist = minkovski_norm(x, point.x, y, point.y);
        if (dist < minimal_dist) {
          color = colors[i % COLORS_AMOUNT];
          minimal_dist = dist;
        }
      }
      image[y][x] = color;
    }
  }
}
int main() {
  srand(time(0));
  generate_seeds();
  // fill_image(COLOR_PINE);
  draw_voronoi_minkovski();
  fill_seeds();
  save_as_ppm("output.ppm");
  return 0;
}
