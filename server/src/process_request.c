#include "../inc/server.h"

void process_request(t_packet *receive_data, t_client *client) {
    if (!receive_data || !receive_data->data) {
        syslog(LOG_ERR, "Invalid t_receive structure in process_request");
        return;
    }

    cJSON *json_payload = cJSON_ParseWithLength(receive_data->data, receive_data->len);
    if (!json_payload) {
        syslog(LOG_ERR, "Failed to parse JSON from received data: %s", receive_data->data);
        return;
    }

    cJSON *request_type = cJSON_GetObjectItemCaseSensitive(json_payload, "request_type");
    if (!cJSON_IsString(request_type) || !request_type->valuestring) {
        syslog(LOG_ERR, "Missing or invalid 'request_type' in JSON data");
        cJSON_Delete(json_payload);
        return;
    }

    syslog(LOG_INFO, "Processing request of type: %s", request_type->valuestring);

    if (strcmp(request_type->valuestring, "login") == 0) {
        handle_login_request(json_payload, client);
    } else if (strcmp(request_type->valuestring, "registration") == 0) {
        handle_register_request(json_payload, client);
    } else if (strcmp(request_type->valuestring, "privateChatCreate") == 0) {
        handle_group_create_request(json_payload, client);
    }

    cJSON_Delete(json_payload);
}

