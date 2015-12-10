#include <stdio.h>
#include <stdlib.h>
#include <gio/gio.h>
#include <eom.h>

/* This test application consider only one output */

static void
test_eom_output_notify_cb_output_add(eom_output_notify_s *notify, void *user_data)
{
	eom_output_notify_add_s *add_notify = (eom_output_notify_add_s*)notify;
	printf("output(%d) connected\n", add_notify->output_id);

	/* already has one connected output */
	if (output)
		return;

	output = eom_output_create(add_notify->output_id);
	if (!output)
		printf("fail: creating output\n");
}

static void
test_eom_output_notify_cb_output_remove(eom_output_notify_s *notify, void *user_data)
{
	eom_output_notify_remove_s *rm_notify = (eom_output_notify_remove_s*)notify;
	printf("output(%d) disconnected\n", rm_notify->output_id);

	/* no connected output */
	if (!output)
		return;

	if (eom_output_get_output_id(output) != rm_notify->output_id) {
		printf("I'm not interested in this output(%d,%d)\n", eom_output_get_output_id(output), rm_notify->output_id);
		return;
	}

	eom_output_destroy(output);
	output = NULL;
}

static void
test_eom_check_connected_output(void)
{
	int output_cnt;
	eom_output_id *output_ids;

	output_ids = eom_get_eom_output_ids(&output_cnt);
	if (!output_ids)
		goto done;

	printf("output(%d) connected\n", output_ids[0]);

done:
	if (output_ids)
		free(output_ids);
}

int
main(int argc, char *argv[])
{
	GMainLoop *event_loop;

	g_type_init();

	if (eom_init())
		return 0;

	eom_output_add_notify_cb(EOM_OUTPUT_NOTIFY_ADD, test_eom_output_notify_cb_output_add, NULL);
	eom_output_add_notify_cb(EOM_OUTPUT_NOTIFY_REMOVE, test_eom_output_notify_cb_output_remove, NULL);

	test_eom_check_connected_output();

	/* run event loop */
	event_loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(event_loop);

	if (output)
		eom_output_destroy(output);

	eom_output_remove_notify_cb(test_eom_output_notify_cb_output_add);
	eom_output_remove_notify_cb(test_eom_output_notify_cb_output_remove);

	eom_deinit();

	return 0;
}
