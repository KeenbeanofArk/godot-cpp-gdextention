#pragma once

#include <godot_cpp/classes/object.hpp>

using namespace godot;

class Summator : public Object {
    GDCLASS(Summator, Object);

    int count;

protected:
    static void _bind_methods();
    // This method is called to notify the object of various events.
    void _notification(int p_what);

public:
    void add(int p_value);
    void reset();
    int get_total() const;
    int get_count() const;
    void set_count(int p_value);

    // Constructor
    Summator();
};
