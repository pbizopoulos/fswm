#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <xcb/xcb.h>

typedef struct Client {
	struct Client *next, *previous;
	xcb_window_t window;
} Client;

int main(int argc, char *argv[]) {
	xcb_generic_event_t *event;
	xcb_connection_t *connection = xcb_connect(NULL, NULL);
	xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
	unsigned int value_list_root[] = {XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT};
	unsigned int value_list_map_request[] = {XCB_EVENT_MASK_STRUCTURE_NOTIFY|XCB_EVENT_MASK_ENTER_WINDOW};
	unsigned int value_list_configure_window[4];
	unsigned int value_list_key_press[] = {XCB_STACK_MODE_ABOVE};
	Client *client_current = (Client *) calloc(1, sizeof(Client));
	client_current->window = xcb_generate_id(connection);
	client_current->next = client_current;
	client_current->previous = client_current;
	signal(SIGCHLD, SIG_IGN);
	if (argc < 2) {;}
	value_list_configure_window[0] = 0;
	value_list_configure_window[1] = 0;
	value_list_configure_window[2] = screen->width_in_pixels;
	value_list_configure_window[3] = screen->height_in_pixels;
	xcb_change_window_attributes(connection, screen->root, XCB_CW_EVENT_MASK, value_list_root);
	xcb_grab_key(connection, 1, screen->root, XCB_MOD_MASK_4, 58, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
	xcb_grab_key(connection, 1, screen->root, XCB_MOD_MASK_4, 44, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
	xcb_grab_key(connection, 1, screen->root, XCB_MOD_MASK_4, 45, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
	while((event = xcb_wait_for_event(connection))) {
		if (event->response_type == XCB_KEY_PRESS) {
			xcb_key_press_event_t *key_press_event = (xcb_key_press_event_t *)event;
			if (key_press_event->detail == 58) {
				if (!(fork()))
					execvp(argv[1], &argv[1]);
			} else if (key_press_event->detail == 44) {
				xcb_configure_window(connection, client_current->next->window, XCB_CONFIG_WINDOW_STACK_MODE, value_list_key_press);
			} else if (key_press_event->detail == 45) {
				xcb_configure_window(connection, client_current->previous->window, XCB_CONFIG_WINDOW_STACK_MODE, value_list_key_press);
			}
		} else if (event->response_type == XCB_MAP_REQUEST) {
			xcb_map_request_event_t *map_request_event = (xcb_map_request_event_t *)event;
			Client *client_new = (Client *) calloc(1, sizeof(Client));
			client_new->window = map_request_event->window;
			client_current->next->previous = client_new;
			client_new->previous = client_current;
			client_new->next = client_current->next;
			client_current->next = client_new;
			xcb_change_window_attributes(connection, client_new->window, XCB_CW_EVENT_MASK, value_list_map_request);
			xcb_configure_window(connection, client_new->window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, value_list_configure_window);
			xcb_map_window(connection, client_new->window);
		} else if (event->response_type == XCB_ENTER_NOTIFY) {
			xcb_enter_notify_event_t *enter_notify_event = (xcb_enter_notify_event_t *)event;
			while (client_current->window != enter_notify_event->event)
				client_current = client_current->next;
			xcb_set_input_focus(connection, XCB_INPUT_FOCUS_POINTER_ROOT, client_current->window, XCB_CURRENT_TIME);
		} else if (event->response_type == XCB_DESTROY_NOTIFY) {
			xcb_destroy_notify_event_t *destroy_notify_event = (xcb_destroy_notify_event_t *)event;
			Client *client_destroyed = client_current;
			while (client_destroyed->window != destroy_notify_event->window)
				client_destroyed = client_destroyed->next;
			if (client_destroyed == client_destroyed->previous)
				client_current = 0;
			if (client_destroyed == client_current)
				client_current = client_destroyed->next;
			if (client_destroyed->next)
				client_destroyed->next->previous = client_destroyed->previous;
			if (client_destroyed->previous)
				client_destroyed->previous->next = client_destroyed->next;
			free(client_destroyed);
		}
		free(event);
		xcb_flush(connection);
	}
	free(client_current);
	xcb_disconnect(connection);
	return 1;
}
