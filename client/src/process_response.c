#include "../inc/client.h"

void process_response(t_packet *recieved_data) {
    if (!recieved_data || !recieved_data->data) {
        fprintf(stderr, "Invalid t_receive structure in process_request\n");
        return;
    }
    
    cJSON *json_payload = cJSON_ParseWithLength(recieved_data->data, recieved_data->len);

    cJSON *response_type = cJSON_GetObjectItemCaseSensitive(json_payload, "response_type");
    if (!cJSON_IsString(response_type) || !response_type->valuestring) {
        fprintf(stderr, "Missing or invalid 'request_type' in JSON data\n");
        cJSON_Delete(json_payload);
        return;
    }

    printf("Processing respond of type: %s\n", response_type->valuestring);

    if (strcmp(response_type->valuestring, "login") == 0) {
        handle_login_response(json_payload);
    } else if (strcmp (response_type->valuestring, "register") == 0) {
        handle_register_response(json_payload);
    } else if (strcmp (response_type->valuestring, "create_group") == 0){
        handle_create_chat_response(json_payload);
    }

    cJSON_Delete(json_payload);
}

