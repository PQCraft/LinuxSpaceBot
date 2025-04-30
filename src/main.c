#include <stdio.h>
#include <signal.h>

#include <concord/discord.h>
#include <concord/log.h>

static struct discord* d;

static void onsignal(int sig) {
    (void)sig;
    discord_shutdown(d);
}

static void onready(struct discord* d, const struct discord_ready* e) {
    (void)d;
    log_info("Connected as %s#%s\n", e->user->username, e->user->discriminator);
}

static void cmd_ping(struct discord* d, const struct discord_message* e) {
    if (e->author->bot) return;
    struct discord_create_message m = {.content = "pong"};
    discord_create_message(d, e->channel_id, &m, NULL);
}

int main(int argc, char** argv) {
    ccord_global_init();
    d = discord_config_init((argc >= 2) ? argv[1] : "config.json");
    if (!d) {
        log_fatal("Failed to init client");
        return 1;
    }
    signal(SIGINT, onsignal);
    signal(SIGTERM, onsignal);

    discord_set_on_ready(d, onready);

    discord_set_prefix(d, "./");
    discord_set_on_command(d, "ping", cmd_ping);

    discord_run(d);

    discord_cleanup(d);
    ccord_global_cleanup();

    return 0;
}
