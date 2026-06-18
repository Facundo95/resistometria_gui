#ifndef SIMPLE_PLOT_H
#define SIMPLE_PLOT_H

#include <FL/Fl_Chart.H>
#include <vector>
#include <string>

struct Tick {
        double value;
        int pixel_pos;
    };

class SimplePlot : public Fl_Chart {
private:
    std::vector<double> x_data;
    std::vector<double> y_data;
    double min_x, max_x, min_y, max_y;
    double display_min_x, display_max_x; // For label drawing logic
    double display_min_y, display_max_y;
    std::string x_axis_label;
    std::string y_axis_label;
    std::vector<Tick> x_ticks;
    std::vector<Tick> y_ticks;
    Fl_Color line_color;

    void update_tick_calculations(); // Logic/Math
    void draw_grid_lines();          // Visuals: Lines
    void draw_data_series();         // Visuals: Data line
    void draw_tick_labels();         // Visuals: Numbers
    void draw_axis_titles();         // Visuals: Axis titles

public:
    // This is the declaration of the constructor
    SimplePlot(int X, int Y, int W, int H, const char* L = 0);

    // Methods
    void add_data(double x, double y);
    void reset();
    void draw() override;
    void set_line_color(Fl_Color c) { line_color = c; redraw(); }
    void set_x_axis_title(const char* title);
    void set_y_axis_title(const char* title);
    void set_axis_titles(const char* x_title, const char* y_title);
    void set_x_axis_label(const char* label) { set_x_axis_title(label); }
    void set_y_axis_label(const char* label) { set_y_axis_title(label); }
    void set_xlimit_start(double start, double end);
};

#endif