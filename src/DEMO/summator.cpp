#include "summator.h"

#include <godot_cpp/core/class_db.hpp>

void Summator::_bind_methods() {
    ClassDB::bind_method(D_METHOD("add", "value"), &Summator::add);
    ClassDB::bind_method(D_METHOD("reset"), &Summator::reset);
    ClassDB::bind_method(D_METHOD("get_total"), &Summator::get_total);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "count", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT), "set_count", "get_count");
}

void Summator::_notification(int p_what)
{
    switch (p_what) {
        case NOTIFICATION_PREDELETE:
            reset();
            break;
        case NOTIFICATION_POSTINITIALIZE:
            // Initialization logic if needed
            break;
        default:
            break;
    }
}

void Summator::add(int p_value) {
    count += p_value;
}

void Summator::reset() {
    count = 0;
}

int Summator::get_total() const {
    return count;
}

int Summator::get_count() const {
    return count;
}

void Summator::set_count(int p_value) {
    count = p_value;
}

Summator::Summator() {
    count = 0;
}
