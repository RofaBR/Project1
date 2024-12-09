#include "../../../../inc/server.h"

int db_group_create(t_group* group)
{
	char* values = NULL;
	asprintf(&values, "'%s', %d, %d, datetime()", group->name, group->is_private, group->created_by);

	int ret = database_create("name, is_private, created_by, created_at", "groups", values);

	mx_strdel(&values);

	return ret;
}

bool db_group_delete(t_group* group)
{
	char* where = NULL;
	if (group->id > 0)asprintf(&where, "id = %d", group->id);

	bool res = database_delete("groups", where);

	mx_strdel(&where);

	return res;
}

bool db_group_delete_by_id(int id)
{
	char* where = NULL;
	asprintf(&where, "id = %d", id);

	bool res = database_delete("groups", where);

	mx_strdel(&where);

	return res;
}

bool db_group_delete_by_named_field(const char* field, const char* value)
{
	char* where = NULL;
	asprintf(&where, "%s = '%s'", field, value);

	bool res = database_delete("groups", where);

	mx_strdel(&where);

	return res;
}

bool db_group_update(t_group* group)
{
	char* where = NULL;
	char* set = NULL;
	asprintf(&where, "id = %d", group->id);
	asprintf(&set, "name = '%s', created_by = %d", group->name, group->created_by);
	
	bool res = database_update("groups", set, where);

	mx_strdel(&where);
	mx_strdel(&set);

	return res;
}

t_group* db_group_read_by_id(int id) {
    syslog(LOG_INFO, "db_group_read_by_id called with id: %d", id);

    char* where = NULL;
    if (asprintf(&where, "groups.id = %d GROUP BY groups.id ORDER BY last_message DESC", id) == -1) {
        syslog(LOG_ERR, "Failed to allocate memory for SQL where clause.");
        return NULL;
    }

    t_list* list = database_read(
        "groups.id, groups.name, groups.is_private, groups.created_by, users.username, groups.created_at, max(messages.created_at) as last_message",
        "groups INNER JOIN users ON groups.created_by = users.id LEFT JOIN messages ON groups.id = group_id",
        where
    );
	
    if (!list) {
        syslog(LOG_ERR, "Database read operation failed for id: %d", id);
        mx_strdel(&where);
        return NULL;
    }

    t_group* ret = group_from_data_list(list);

    if (!ret) {
        syslog(LOG_WARNING, "No group data found for id: %d", id);
    } else {
        syslog(LOG_INFO, "Successfully retrieved group data for id: %d", id);
    }

    mx_del_list(list, mx_list_size(list));
    mx_strdel(&where);

    syslog(LOG_INFO, "db_group_read_by_id completed for id: %d", id);

    return ret;
}

t_list* db_group_read_all(void) {

	t_list* list = database_read("groups.id, groups.name, groups.is_private, groups.created_by, users.username, groups.created_at, max(messages.created_at) as last_message", "groups INNER JOIN users ON groups.created_by = users.id INNER JOIN messages ON groups.id = group_id GROUP BY groups.id ORDER BY last_message DESC", NULL);
	t_list* ret = group_list_from_data_list(list);

	mx_del_list(list, mx_list_size(list));

	return ret;
}

bool db_private_group_exists(int idA, int idB) {
	char* from = NULL;
	char* where = NULL;
	asprintf(&from, "groups INNER JOIN users_groups ON users_groups.group_id = groups.id INNER JOIN (SELECT group_id as gid FROM users_groups WHERE user_id = %d) as A ON users_groups.group_id = A.gid", idA);
	asprintf(&where, "users_groups.user_id = %d AND groups.is_private = 1", idB);
	t_list* list = database_read("users_groups.id", from, where);

	mx_strdel(&from);
	mx_strdel(&where);

	if (!list) return false;

	int size = mx_list_size(list);
	t_list* current = list;
	while (current) {
		free(current->data);
		current = current->next;
	}
	mx_del_list(list, size);

	return true;
}
