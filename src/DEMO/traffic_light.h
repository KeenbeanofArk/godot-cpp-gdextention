#pragma once

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>

using namespace godot;

enum TrafficLightState {
    STOP,
    CAUTION,
    GO
};

class TrafficLight : public Control {
    GDCLASS(TrafficLight, Control);

    TextureRect *texture_rect;

    Ref<Texture2D> stop_texture;
    Ref<Texture2D> caution_texture;
    Ref<Texture2D> go_texture;

    TrafficLightState current_state;

protected:
    static void _bind_methods();

    void _notification(int p_what);

public:
    void set_stop_texture(const Ref<Texture2D> &p_texture);
    Ref<Texture2D> get_stop_texture() const;

    void set_caution_texture(const Ref<Texture2D> &p_texture);
    Ref<Texture2D> get_caution_texture() const;

    void set_go_texture(const Ref<Texture2D> &p_texture);
    Ref<Texture2D> get_go_texture() const;

    void set_current_state(TrafficLightState p_state);
    TrafficLightState get_current_state() const;

    virtual void show_next_light();

    GDVIRTUAL1RC(TrafficLightState, get_current_state, TrafficLightState);

    TrafficLight();
};

VARIANT_ENUM_CAST(TrafficLightState);