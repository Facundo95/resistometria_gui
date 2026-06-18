#include "simple_plot.h"
#include <iomanip>
#include <sstream>
#include <FL/fl_draw.H>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace {
    double snap_tick_step(double range, int target_tick_count) {
        if (target_tick_count < 1) target_tick_count = 1;

        if (range <= 0.0) return 1.0;

        double raw_step = range / static_cast<double>(target_tick_count);
        if (raw_step <= 0.0) raw_step = 1.0;

        double magnitude = std::pow(10.0, std::floor(std::log10(raw_step)));
        double normalized = raw_step / magnitude;

        double nice_normalized = 1.0;
        if (normalized <= 1.0) nice_normalized = 1.0;
        else if (normalized <= 2.0) nice_normalized = 2.0;
        else if (normalized <= 5.0) nice_normalized = 5.0;
        else nice_normalized = 10.0;

        return nice_normalized * magnitude;
    }

    double snap_down(double value, double step) {
        return std::floor(value / step) * step;
    }

    double snap_up(double value, double step) {
        return std::ceil(value / step) * step;
    }

    void format_tick_value(double value, int exp, char* out) {
        double scaled = value / std::pow(10.0, exp);
        if (std::abs(scaled) < 1e-9) {
            sprintf(out, "0");
        } else {
            sprintf(out, "%.2f", scaled);
        }
    }
}

SimplePlot::SimplePlot(int X, int Y, int W, int H, const char* L) 
    : Fl_Chart(X, Y, W, H, 0) {

    type(FL_LINE_CHART);  // Connects points with lines
    box(FL_FLAT_BOX);
    color(FL_WHITE);      // Background
    selection_color(FL_WHITE);

    reset();
}

void SimplePlot::set_x_axis_title(const char* title) {
    x_axis_label = title ? title : "";
    redraw();
}

void SimplePlot::set_y_axis_title(const char* title) {
    y_axis_label = title ? title : "";
    redraw();
}

void SimplePlot::set_axis_titles(const char* x_title, const char* y_title) {
    x_axis_label = x_title ? x_title : "";
    y_axis_label = y_title ? y_title : "";
    redraw();
}

void SimplePlot::set_xlimit_start(double start, double end) {
    display_min_x = start;
    display_max_x = end;
    update_tick_calculations();
    redraw();
}

void SimplePlot::reset() {
    clear();
    x_data.clear();
    y_data.clear();
    min_x = 1e30;
    max_x = -1e30;
    min_y = 1e30;
    max_y = -1e30;
    display_min_x = 0.0;
    display_max_x = 1.0;
    display_min_y = 0.0;
    display_max_y = 1.0;
    x_ticks.clear();
    y_ticks.clear();
    line_color = FL_BLUE;
    bounds(display_min_y, display_max_y); // Default initial view
}

void SimplePlot::add_data(double x, double y) {
    x_data.push_back(x);
    y_data.push_back(y);

    // 1. Update the actual data limits
    if (x < min_x) min_x = x;
    if (x > max_x) max_x = x;
    if (y < min_y) min_y = y;
    if (y > max_y) max_y = y;

    // 2. Calculate the "Real" Y-range used by the chart (including your 10% padding)
    double data_range_y = max_y - min_y;
    if (data_range_y == 0) data_range_y = 1.0; 

    // 4. Keep the displayed X-axis starting from the first data value
    this->display_min_x = min_x;
    this->display_max_x = max_x;

    // 5. Apply Y-bounds to Fl_Chart
    double chart_min_y = min_y - (data_range_y * 0.1);
    double chart_max_y = max_y + (data_range_y * 0.1);
    bounds(chart_min_y, chart_max_y);
    
    redraw();
    std::cout << "Added point: (" << x << ", " << y << ")\n";
}
    
void SimplePlot::update_tick_calculations() {
    x_ticks.clear();
    y_ticks.clear();

    if (x_data.empty() || y_data.empty()) {
        double active_range_y = display_max_y - display_min_y;
        if (active_range_y <= 0.0) {
            display_min_y = 0.0;
            display_max_y = 1.0;
            active_range_y = 1.0;
        }

        bounds(display_min_y, display_max_y);

        int num_y = h() / 40;
        if (num_y < 2) num_y = 2;

        double y_step = snap_tick_step(active_range_y, num_y);
        double y_start = snap_down(display_min_y, y_step);
        double y_end = snap_up(display_max_y, y_step);
        active_range_y = y_end - y_start;
        if (active_range_y <= 0.0) active_range_y = y_step;

        int y_tick_count = static_cast<int>(std::round(active_range_y / y_step));
        if (y_tick_count < 1) y_tick_count = 1;

        for (int i = 0; i <= y_tick_count; ++i) {
            double value = y_start + (y_step * i);
            Tick t;
            t.value = value;
            double normalized = (value - y_start) / active_range_y;
            t.pixel_pos = y() + h() - static_cast<int>(h() * normalized);
            y_ticks.push_back(t);
        }

        double active_range_x = display_max_x - display_min_x;
        if (active_range_x <= 0.0) {
            display_min_x = 0.0;
            display_max_x = 1.0;
            active_range_x = 1.0;
        }

        double aspect_ratio = (double)w() / (double)h();
        int num_x = (int)(num_y * aspect_ratio);
        if (num_x < 2) num_x = 2;

        double x_step = snap_tick_step(active_range_x, num_x);
        double x_start = snap_down(display_min_x, x_step);
        double x_end = snap_up(display_max_x, x_step);
        active_range_x = x_end - x_start;
        if (active_range_x <= 0.0) active_range_x = x_step;

        int x_tick_count = static_cast<int>(std::round(active_range_x / x_step));
        if (x_tick_count < 1) x_tick_count = 1;

        display_min_x = x_start;
        display_max_x = x_end;

        for (int i = 0; i <= x_tick_count; ++i) {
            double value = x_start + (x_step * i);
            Tick t;
            t.value = value;
            double normalized = (value - x_start) / active_range_x;
            t.pixel_pos = x() + static_cast<int>(w() * normalized);
            x_ticks.push_back(t);
        }

        return;
    }

    // 1. Calculate Y-Range (with 10% padding), and keep view clipped to it
    double data_range_y = max_y - min_y;
    if (data_range_y <= 0.0) {
        double base = std::max(std::abs(min_y), 1e-6);
        data_range_y = base * 0.2;
    }
    double y_padding = data_range_y * 0.1;
    double d_min_y = min_y - y_padding;
    double d_max_y = max_y + y_padding;

    display_min_y = d_min_y;
    display_max_y = d_max_y;
    bounds(display_min_y, display_max_y);

    // 2. Calculate Y-Ticks
    int num_y = h() / 40;
    if (num_y < 2) num_y = 2;

    double y_step = snap_tick_step(d_max_y - d_min_y, num_y);
    double y_start = snap_down(d_min_y, y_step);
    double y_end = snap_up(d_max_y, y_step);
    double active_range_y = y_end - y_start;
    if (active_range_y <= 0.0) active_range_y = y_step;

    int y_tick_count = static_cast<int>(std::round(active_range_y / y_step));
    if (y_tick_count < 1) y_tick_count = 1;

    for (int i = 0; i <= y_tick_count; ++i) {
        double value = y_start + (y_step * i);
        Tick t;
        t.value = value;
        double normalized = (value - y_start) / active_range_y;
        t.pixel_pos = y() + h() - static_cast<int>(h() * normalized);
        y_ticks.push_back(t);
    }

    // 3. Calculate 1:1 X-Range based on displayed Y-Range and widget aspect ratio
    double aspect_ratio = (double)w() / (double)h();

    this->display_min_x = min_x;
    this->display_max_x = max_x;
    double active_range_x = display_max_x - display_min_x;
    if (active_range_x <= 0.0) active_range_x = 1.0;

    // 4. Calculate X-Ticks (matching the density of Y)
    int num_x = (int)(num_y * aspect_ratio);
    if (num_x < 2) num_x = 2;

    double x_step = snap_tick_step(active_range_x, num_x);
    double x_start = snap_down(display_min_x, x_step);
    double x_end = snap_up(display_max_x, x_step);
    active_range_x = x_end - x_start;
    if (active_range_x <= 0.0) active_range_x = x_step;

    int x_tick_count = static_cast<int>(std::round(active_range_x / x_step));
    if (x_tick_count < 1) x_tick_count = 1;

    display_min_x = x_start;
    display_max_x = x_end;

    for (int i = 0; i <= x_tick_count; ++i) {
        double value = x_start + (x_step * i);
        Tick t;
        t.value = value;
        double normalized = (value - x_start) / active_range_x;
        t.pixel_pos = x() + static_cast<int>(w() * normalized);
        x_ticks.push_back(t);
    }

    // Calcula el exponente del tick de mayor magnitud en un vector
    auto calc_exp = [](const std::vector<Tick>& ticks) -> int {
        double max_abs = 0.0;
        for (const auto& t : ticks)
            max_abs = std::max(max_abs, std::abs(t.value));
        if (max_abs == 0.0) return 0;
        return (int)std::floor(std::log10(max_abs));
    };

    y_exp = calc_exp(y_ticks);
    x_exp = calc_exp(x_ticks);
}

void SimplePlot::draw_grid_lines() {
    fl_color(FL_BLACK); // Light gray for grid
    fl_line_style(FL_DOT);

    for (const auto& t : x_ticks) {
        if (t.pixel_pos >= x() && t.pixel_pos <= x() + w())
            fl_line(t.pixel_pos, y(), t.pixel_pos, y() + h());
    }
    for (const auto& t : y_ticks) {
        fl_line(x(), t.pixel_pos, x() + w(), t.pixel_pos);
    }
    fl_line_style(0);
}

void SimplePlot::draw_data_series() {
    if (x_data.size() < 2 || y_data.size() < 2) return;

    double range_x = display_max_x - display_min_x;
    double range_y = display_max_y - display_min_y;
    if (range_x <= 0.0 || range_y <= 0.0) return;

    auto map_x = [&](double value) {
        double normalized = (value - display_min_x) / range_x;
        return x() + static_cast<int>(std::round(normalized * w()));
    };

    auto map_y = [&](double value) {
        double normalized = (value - display_min_y) / range_y;
        return y() + h() - static_cast<int>(std::round(normalized * h()));
    };

    fl_push_clip(x(), y(), w(), h());
    fl_color(line_color);
    fl_line_style(FL_SOLID, 2);

    for (size_t i = 1; i < x_data.size(); ++i) {
        int x1 = map_x(x_data[i - 1]);
        int y1 = map_y(y_data[i - 1]);
        int x2 = map_x(x_data[i]);
        int y2 = map_y(y_data[i]);
        fl_line(x1, y1, x2, y2);
    }

    fl_line_style(0);
    fl_pop_clip();
}

void SimplePlot::draw_tick_labels() {
    fl_font(FL_HELVETICA, 10);
    fl_color(FL_BLACK);
    char buf[64];

    int y_label_right_x = x() - 8;

    // Y-Axis Labels
    for (const auto& t : y_ticks) {
        format_tick_value(t.value, y_exp, buf);

        int tw = 0;
        int th = 0;
        fl_measure(buf, tw, th, 0);
        int label_x = std::max(2, y_label_right_x - tw);

        fl_draw(buf, label_x, t.pixel_pos + 4);
        fl_line(x() - 5, t.pixel_pos, x(), t.pixel_pos);
    }

    // X-Axis Labels
    for (const auto& t : x_ticks) {
        if (t.pixel_pos < x() || t.pixel_pos > x() + w()) continue;

        format_tick_value(t.value, x_exp, buf);

        int tw, th;
        fl_measure(buf, tw, th);
        fl_draw(buf, t.pixel_pos - (tw / 2), y() + h() + 15);
        fl_line(t.pixel_pos, y() + h(), t.pixel_pos, y() + h() + 5);
    }

    // Multiplicador eje Y: arriba del eje, alineado a la izquierda del área de ticks
    if (y_exp != 0) {
        fl_font(FL_HELVETICA, 10);
        char exp_buf[32];
        sprintf(exp_buf, "\xc3\x97" "10^ %d", y_exp); // "×10^n" en UTF-8
        fl_color(FL_BLACK);
        fl_draw(exp_buf, x() + 4, y() - 4);
    }

    // Multiplicador eje X: a la derecha del último tick
    if (x_exp != 0) {
        fl_font(FL_HELVETICA, 10);
        char exp_buf[32];
        sprintf(exp_buf, "\xc3\x97" "10^ %d", x_exp);
        fl_color(FL_BLACK);
        fl_draw(exp_buf, x() + w() + 4, y() + h() - 4);
    }
}

void SimplePlot::draw_axis_titles() {
    fl_font(FL_HELVETICA_BOLD, 12);
    fl_color(FL_BLACK);

    if (!x_axis_label.empty()) {
        int tw = 0;
        int th = 0;
        fl_measure(x_axis_label.c_str(), tw, th, 0);
        fl_draw(x_axis_label.c_str(), x() + (w() - tw) / 2, y() + h() + 40);
    }

    if (!y_axis_label.empty()) {
        int max_y_tick_width = 0;
        char tick_buf[64];
        fl_font(FL_HELVETICA, 10);
        for (const auto& t : y_ticks) {
            format_tick_value(t.value, y_exp, tick_buf);

            int tick_w = 0;
            int tick_h = 0;
            fl_measure(tick_buf, tick_w, tick_h, 0);
            if (tick_w > max_y_tick_width) max_y_tick_width = tick_w;
        }

        fl_font(FL_HELVETICA_BOLD, 12);
        int title_width = 0;
        int title_height = 0;
        fl_measure(y_axis_label.c_str(), title_width, title_height, 0);

        int y_label_right_x = x() - 8;
        int title_x = std::max(8, y_label_right_x - max_y_tick_width - 18);
        int title_y = y() + (h() / 2) + (title_width / 2);

        fl_draw(90, y_axis_label.c_str(), title_x, title_y);
    }
}

void SimplePlot::draw() {
    Fl_Chart::draw(); // Base plot
    
    update_tick_calculations(); // Refresh math
    
    fl_push_no_clip();

    int max_y_tick_width = 0;
    char tick_buf[64];
    fl_font(FL_HELVETICA, 10);
    for (const auto& t : y_ticks) {
        format_tick_value(t.value, y_exp, tick_buf);
        int tick_w = 0;
        int tick_h = 0;
        fl_measure(tick_buf, tick_w, tick_h, 0);
        if (tick_w > max_y_tick_width) max_y_tick_width = tick_w;
    }

    int y_title_height = 0;
    if (!y_axis_label.empty()) {
        fl_font(FL_HELVETICA_BOLD, 12);
        int title_w = 0;
        int title_h = 0;
        fl_measure(y_axis_label.c_str(), title_w, title_h, 0);
        y_title_height = title_h;
    }

    int left_margin_width = max_y_tick_width + y_title_height + 46;
    if (left_margin_width < 80) left_margin_width = 80;
    int left_margin_x = x() - left_margin_width;
    
    // Clear the outside area before drawing new labels
    fl_color(FL_GRAY);
    fl_rectf(left_margin_x, y() - 50, left_margin_width - 2, h() + 90); // Y area
    fl_rectf(x() - 20, y() + h(), w() + 120, 80); // X area
    fl_rectf(x() + w(), y() - 20, 120, h() + 40); // Right area for X multiplier
    fl_rectf(left_margin_x, y() - 60, left_margin_width + 90, h() - 100); // Left area for Y multiplier

    draw_grid_lines();
    draw_data_series();
    draw_tick_labels();
    draw_axis_titles();

    fl_pop_clip();
}