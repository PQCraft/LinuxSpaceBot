#include <stdio.h>
#include <signal.h>

#include <concord/discord.h>
#include <concord/log.h>

static struct discord* d;

static u64snowflake chid_members;
static u64snowflake chid_msglog;

static void onsignal(int sig) {
    (void)sig;
    discord_shutdown(d);
}

static void on_ready(struct discord* d, const struct discord_ready* e) {
    (void)d;
    log_info("Connected as %s#%s (%llu)", e->user->username, e->user->discriminator, (long long unsigned)e->user->id);
}

static void on_membjoin(struct discord* d, const struct discord_guild_member* e) {
    char buf[64];
    sprintf(buf, "<@!%llu> has joined!", (long long unsigned)e->user->id);
    struct discord_create_message m = {.content = buf};
    discord_create_message(d, chid_members, &m, NULL);
}
static void on_membleave(struct discord* d, const struct discord_guild_member_remove* e) {
    char buf[64];
    sprintf(buf, "<@!%llu> has left.", (long long unsigned)e->user->id);
    struct discord_create_message m = {.content = buf};
    discord_create_message(d, chid_members, &m, NULL);
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

    discord_set_on_ready(d, on_ready);

    chid_members = (u64snowflake)strtoumax(discord_config_get_field(d, (char*[2]){"channels", "members"}, 2).start, NULL, 10);
    log_info("Members channel: %llu", (long long unsigned)chid_members);
    chid_msglog = (u64snowflake)strtoumax(discord_config_get_field(d, (char*[2]){"channels", "msglog"}, 2).start, NULL, 10);
    log_info("Message log channel: %llu", (long long unsigned)chid_msglog);

    discord_set_on_guild_member_add(d, on_membjoin);
    discord_set_on_guild_member_remove(d, on_membleave);

    discord_set_prefix(d, "./");
    discord_set_on_command(d, "ping", cmd_ping);

    discord_run(d);

    discord_cleanup(d);
    ccord_global_cleanup();

    return 0;
}
