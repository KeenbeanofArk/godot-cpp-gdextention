#include "traffic_light.h"

#include <godot_cpp/core/class_db.hpp>

void TrafficLight::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_stop_texture", "texture"), &TrafficLight::set_stop_texture);
    ClassDB::bind_method(D_METHOD("get_stop_texture"), &TrafficLight::get_stop_texture);
    ClassDB::bind_method(D_METHOD("set_caution_texture", "texture"), &TrafficLight::set_caution_texture);
    ClassDB::bind_method(D_METHOD("get_caution_texture"), &TrafficLight::get_caution_texture);
    ClassDB::bind_method(D_METHOD("set_go_texture", "texture"), &TrafficLight::set_go_texture);
    ClassDB::bind_method(D_METHOD("get_go_texture"), &TrafficLight::get_go_texture);

    ClassDB::bind_method(D_METHOD("set_current_state", "state"), &TrafficLight::set_current_state);
    ClassDB::bind_method(D_METHOD("get_current_state"), &TrafficLight::get_current_state);

    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "stop_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_stop_texture", "get_stop_texture");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "caution_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_caution_texture", "get_caution_texture");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "go_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_go_texture", "get_go_texture");

    ADD_PROPERTY(PropertyInfo(Variant::INT, "current_state", PROPERTY_HINT_ENUM, "STOP,CAUTION,GO"), "set_current_state", "get_current_state");

    BIND_ENUM_CONSTANT(STOP);
    BIND_ENUM_CONSTANT(CAUTION);
    BIND_ENUM_CONSTANT(GO);

    GDVIRTUAL_BIND(get_current_state);
}

void TrafficLight::_notification(int p_what){
    switch (p_what) {
        case NOTIFICATION_READY: {
            set_current_state(current_state);
            break;
        }
        default:
            break;
    }
};

void TrafficLight::set_stop_texture(const Ref<Texture2D> &p_texture){
    stop_texture = p_texture;
    texture_rect->set_texture(stop_texture);

};

Ref<Texture2D> TrafficLight::get_stop_texture() const{
    return stop_texture;
};

void TrafficLight::set_caution_texture(const Ref<Texture2D> &p_texture){
    caution_texture = p_texture;
    texture_rect->set_texture(caution_texture);
};

Ref<Texture2D> TrafficLight::get_caution_texture() const{
    return caution_texture;
};

void TrafficLight::set_go_texture(const Ref<Texture2D> &p_texture){
    go_texture = p_texture;
    texture_rect->set_texture(go_texture);
};

Ref<Texture2D> TrafficLight::get_go_texture() const{
    return go_texture;
};

void TrafficLight::set_current_state(TrafficLightState state) {
    current_state = state;

    switch (current_state) {
        case STOP:
            texture_rect->set_texture(stop_texture);
            break;
        case CAUTION:
            texture_rect->set_texture(caution_texture);
            break;
        case GO:
            texture_rect->set_texture(go_texture);
            break;
       
    }
}

TrafficLightState TrafficLight::get_current_state() const {
    return current_state;
}

void TrafficLight::show_next_light() {
    TrafficLightState next_state;
    if(GDVIRTUAL_CALL(get_current_state, light_type, next_state)) {
       set_current_state(next_state);
    }
}   

TrafficLight::TrafficLight() {
    texture_rect = memnew(TextureRect);
    texture_rect->set_name("TextureRect");
    add_child(texture_rect);
    texture_rect->set_anchors_preset(PRESET_FULL_RECT);
    current_state = STOP;
}