#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/keysym.h>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

typedef struct List {
	struct Client *head;
	struct Client *tail;
} List;

typedef struct Client {
	struct Client *next;
	struct Client *previous;
	struct List *parent;
	xcb_window_t window;
} Client;

static void update_client(Client *client, xcb_connection_t *connection);

static Client *client_current = NULL;
static Client *client_previous_focus = NULL;
static List clients;

static Client *remove_client(Client *client) {
	if (client == client->parent->head) {
		client->parent->head = client->parent->head->next;
		if (client->parent->head) {
			client->parent->head->previous = NULL;
		} else {
			client->parent->tail = NULL;
		}
	} else if (client == client->parent->tail) {
		client->parent->tail = client->parent->tail->previous;
		client->parent->tail->next = NULL;
	} else {
		client->previous->next = client->next;
		client->next->previous = client->previous;
	}
	client->previous = client->next = NULL;
	client->parent = NULL;
	return client;
}

void update_client(Client *client_focus, xcb_connection_t *connection) {
	unsigned int key_press_value_list[] = {XCB_STACK_MODE_ABOVE};
	Client *client_head = clients.head;
	if (client_focus) {
		if (client_focus == client_previous_focus) {
			client_current = client_previous_focus;
			client_previous_focus = client_current->previous;
		} else {
			client_previous_focus = client_current;
			client_current = client_focus;
		}
	} else {
		if (client_previous_focus) {
			client_current = client_previous_focus;
		} else {
			client_current = clients.head;
		}
		if (client_current) {
			client_previous_focus = client_current->previous;
		} else {
			client_previous_focus = NULL;
		}
	}
	if (!client_current) {
		return;
	}
	if (!clients.head) {
		return;
	}
	while (client_head) {
		if (client_head == client_current) {
			xcb_configure_window(connection, client_head->window, XCB_CONFIG_WINDOW_STACK_MODE, key_press_value_list);
		}
		client_head = client_head->next;
	}
	xcb_set_input_focus(connection, XCB_INPUT_FOCUS_POINTER_ROOT, client_current->window, XCB_CURRENT_TIME);
}

int main(int argc, char *argv[]) {
	Client *client = calloc(1, sizeof(Client));
	unsigned int map_request_configure_value_list[4];
	unsigned int root_value_list[] = {XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT|XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY|XCB_EVENT_MASK_PROPERTY_CHANGE};
	xcb_connection_t *connection = xcb_connect(NULL, NULL);
	xcb_generic_event_t *generic_event = NULL;
	xcb_key_symbols_t *key_symbols = xcb_key_symbols_alloc(connection);
	xcb_keycode_t *delete_keycode = xcb_key_symbols_get_keycode(key_symbols, XK_Delete);
	xcb_keycode_t *t_keycode = xcb_key_symbols_get_keycode(key_symbols, XK_T);
	xcb_keycode_t *tab_keycode = xcb_key_symbols_get_keycode(key_symbols, XK_Tab);
	xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
	xcb_key_symbols_free(key_symbols);
	signal(SIGCHLD, SIG_IGN);
	if (argc < 2) {}
	map_request_configure_value_list[0] = 0;
	map_request_configure_value_list[1] = 0;
	map_request_configure_value_list[2] = screen->width_in_pixels;
	map_request_configure_value_list[3] = screen->height_in_pixels;
	xcb_change_window_attributes(connection, screen->root, XCB_CW_EVENT_MASK, root_value_list);
	xcb_grab_key(connection, 1, screen->root, XCB_MOD_MASK_1, *tab_keycode, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
	xcb_grab_key(connection, 1, screen->root, XCB_MOD_MASK_1|XCB_MOD_MASK_SHIFT, *tab_keycode, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
	xcb_grab_key(connection, 1, screen->root, XCB_MOD_MASK_CONTROL|XCB_MOD_MASK_1, *t_keycode, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
	xcb_grab_key(connection, 1, screen->root, XCB_MOD_MASK_CONTROL|XCB_MOD_MASK_1, *delete_keycode, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
	while (1) {
		xcb_flush(connection);
		generic_event = xcb_wait_for_event(connection);
		if (generic_event->response_type == XCB_KEY_PRESS) {
			const xcb_key_press_event_t *key_press_event = (xcb_key_press_event_t *)generic_event;
			if (key_press_event->detail == *tab_keycode && key_press_event->state == (XCB_MOD_MASK_1|XCB_MOD_MASK_SHIFT)) {
				Client *client_focus = NULL;
				if (client_current) {
					client_focus = client_current->previous;
				} else {
					client_focus = NULL;
				}
				if (!(client_focus)) {
					client_focus = clients.tail;
				}
				client_previous_focus = client_current;
				update_client(client_focus, connection);
			} else if (key_press_event->detail == *tab_keycode) {
				Client *client_focus = NULL;
				if (client_current) {
					client_focus = client_current->next;
				} else {
					client_focus = NULL;
				}
				if (!(client_focus)) {
					client_focus = clients.head;
				}
				client_previous_focus = client_current;
				update_client(client_focus, connection);
			} else if (key_press_event->detail == *delete_keycode) {
				free(generic_event);
				break;
			} else if (key_press_event->detail == *t_keycode) {
				if (!(fork())) {
					execvp(argv[1], &argv[1]);
				}
			}
		} else if (generic_event->response_type == XCB_MAP_REQUEST) {
			const xcb_map_request_event_t *map_request_event = (xcb_map_request_event_t *)generic_event;
			Client *client_map_request = NULL;
			if (clients.head) {
				client_map_request = clients.head;
			} else {
				client_map_request = NULL;
			}
			while (client_map_request) {
				if (client_map_request->window == map_request_event->window) {
					break;
				}
				client_map_request = client_map_request->next;
			}
			if (!(client_map_request)) {
				Client *client_new = calloc(1, sizeof(Client));
				client_new->window = map_request_event->window;
				if (clients.head) {
					Client *clients_tail = clients.tail;
					clients.tail = client_new;
					clients_tail->next = client_new;
					client_new->previous = clients_tail;
					client_new->next = NULL;
				} else {
					clients.head = client_new;
					clients.tail = client_new;
				}
				client_new->parent = &clients;
				client_map_request = client_new;
			}
			xcb_map_window(connection, client_map_request->window);
			update_client(client_map_request, connection);
			xcb_configure_window(connection, client_current->window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, map_request_configure_value_list);
		} else if (generic_event->response_type == XCB_UNMAP_NOTIFY) {
			const xcb_unmap_notify_event_t *unmap_notify_event = (xcb_unmap_notify_event_t *)generic_event;
			Client *client_unmap_notify = NULL;
			if (clients.head) {
				client_unmap_notify = clients.head;
			} else {
				client_unmap_notify = NULL;
			}
			while (client_unmap_notify) {
				if (client_unmap_notify->window == unmap_notify_event->window) {
					break;
				}
				client_unmap_notify = client_unmap_notify->next;
			}
			if (!client_unmap_notify) {
				continue;
			}
			remove_client(client_unmap_notify);
			if (client_unmap_notify == client_previous_focus) {
				if (client_current) {
					client_previous_focus = client_current->previous;
				} else {
					client_previous_focus = NULL;
				}
			}
			update_client(client_previous_focus, connection);
			free(client_unmap_notify);
			client_unmap_notify = NULL;
		}
		free(generic_event);
	}
	while (client) {
		free(client);
		if (clients.head) {
			client = remove_client(clients.head);
		} else {
			client = NULL;
		}
	}
	xcb_disconnect(connection);
	free(delete_keycode);
	free(t_keycode);
	free(tab_keycode);
	return 0;
}
