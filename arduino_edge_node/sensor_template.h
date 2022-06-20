class SensorConfig
{{  // template requires double brackets, otherwise Python cannot format this string properly
public:
    const char mqtt_data_topic[100] = "sensors/{SENSOR_ID}/data";
    const char mqtt_description_topic[100] = "sensors/{SENSOR_ID}/description";
    const char sensor_self_description[10000] = R"###({SENSOR_DESCRIPTION})###";
}};