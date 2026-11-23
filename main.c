#include <stdbool.h>
#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>

#define SIMULATION_ATTEMPTS 10000
#define WEEKS_PER_TRACK 3
#define TRACKS_PER_ALBUM 3

struct WeekMetrics {
    int unique_listeners;
    int streams;
    int saves;
    int new_followers;
    int revenue_usd;
    int ad_spend_usd;
};

struct Track {
    char title[32];
    int recording_cost_usd;
    struct WeekMetrics weeks[WEEKS_PER_TRACK];
};

struct Album {
    char title[32];
    int track_count;
    struct Track tracks[TRACKS_PER_ALBUM];
};

struct TrackReport {
    int listener_periods;
    int total_streams;
    int total_saves;
    int total_followers;
    int total_revenue_usd;
    int total_ad_spend_usd;
    int net_income_usd;
    double save_rate_percent;
    double follower_rate_percent;
    double average_weekly_stream_change;
    double next_week_stream_forecast;
    double average_weekly_revenue_change;
    double next_week_revenue_forecast;
};

bool got_invitation(int social_status, int invitation_check)
{
    int invitation_limit;

    if (social_status <= 3) {
        invitation_limit = 1;
    } else if (social_status <= 7) {
        invitation_limit = 4;
    } else {
        invitation_limit = 7;
    }

    return invitation_check <= invitation_limit;
}

int minimum_fee_for_loyalty(int manager_loyalty)
{
    if (manager_loyalty <= 3) {
        return 200;
    } else if (manager_loyalty <= 7) {
        return 150;
    } else {
        return 100;
    }
}

int social_bonus_for_status(int social_status)
{
    if (social_status <= 3) {
        return 0;
    } else if (social_status <= 7) {
        return 50;
    } else {
        return 100;
    }
}

struct TrackReport analyze_track(struct Track *track)
{
    struct TrackReport report = {0};

    for (int week = 0; week < WEEKS_PER_TRACK; week++) {
        report.listener_periods += track->weeks[week].unique_listeners;
        report.total_streams += track->weeks[week].streams;
        report.total_saves += track->weeks[week].saves;
        report.total_followers += track->weeks[week].new_followers;
        report.total_revenue_usd += track->weeks[week].revenue_usd;
        report.total_ad_spend_usd += track->weeks[week].ad_spend_usd;
    }

    report.net_income_usd =
        report.total_revenue_usd -
        track->recording_cost_usd -
        report.total_ad_spend_usd;

    if (report.listener_periods > 0) {
        report.save_rate_percent =
            100.0 * report.total_saves / report.listener_periods;
        report.follower_rate_percent =
            100.0 * report.total_followers / report.listener_periods;
    }

    int first_week = 0;
    int last_week = WEEKS_PER_TRACK - 1;
    int intervals = WEEKS_PER_TRACK - 1;

    report.average_weekly_stream_change =
        (double)(track->weeks[last_week].streams -
                 track->weeks[first_week].streams) / intervals;
    report.next_week_stream_forecast =
        track->weeks[last_week].streams +
        report.average_weekly_stream_change;

    report.average_weekly_revenue_change =
        (double)(track->weeks[last_week].revenue_usd -
                 track->weeks[first_week].revenue_usd) / intervals;
    report.next_week_revenue_forecast =
        track->weeks[last_week].revenue_usd +
        report.average_weekly_revenue_change;

    if (report.next_week_stream_forecast < 0) {
        report.next_week_stream_forecast = 0;
    }

    if (report.next_week_revenue_forecast < 0) {
        report.next_week_revenue_forecast = 0;
    }

    return report;
}

void print_album_report(struct Album *album)
{
    printf("\nКаталог: %s\n", album->title);

    for (int i = 0; i < album->track_count; i++) {
        struct Track *track = &album->tracks[i];
        struct TrackReport report = analyze_track(track);

        printf("\nТрек: %s\n", track->title);
        printf("Слушатели по неделям: %d\n", report.listener_periods);
        printf("Прослушивания за три недели: %d\n", report.total_streams);
        printf("Сохранения: %d, новые подписчики: %d\n",
               report.total_saves, report.total_followers);
        printf("Доход: $%d, реклама: $%d, запись: $%d\n",
               report.total_revenue_usd,
               report.total_ad_spend_usd,
               track->recording_cost_usd);
        printf("Чистый доход: $%d\n", report.net_income_usd);
        printf("Доля сохранений: %.1f%%, конверсия в подписку: %.1f%%\n",
               report.save_rate_percent,
               report.follower_rate_percent);
        printf("Среднее изменение прослушиваний за неделю: %.1f\n",
               report.average_weekly_stream_change);
        printf("Прогноз прослушиваний на следующую неделю: %.1f\n",
               report.next_week_stream_forecast);
        printf("Прогноз дохода на следующую неделю: $%.1f\n",
               report.next_week_revenue_forecast);
    }
}

int main(void)
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    srand((unsigned)time(NULL));

    struct Album album = {
        "Демо-релиз",
        TRACKS_PER_ALBUM,
        {
            {
                "Сигнал",
                400,
                {
                    {10000, 15000, 750, 300, 45, 100},
                    {13000, 21000, 1200, 500, 63, 80},
                    {16000, 27000, 1800, 750, 81, 60}
                }
            },
            {
                "После полуночи",
                300,
                {
                    {8000, 12000, 400, 140, 36, 50},
                    {8500, 13000, 420, 150, 39, 40},
                    {8200, 12500, 410, 130, 37, 30}
                }
            },
            {
                "Статика",
                350,
                {
                    {5000, 7000, 210, 80, 21, 30},
                    {9000, 13000, 650, 250, 39, 50},
                    {18000, 28000, 2000, 950, 84, 120}
                }
            }
        }
    };

    print_album_report(&album);

    int manager_loyalty;

    printf("\nЛояльность менеджеру (0-10): ");

    if (scanf("%d", &manager_loyalty) != 1) {
        printf("Ошибка: введите целое число.\n");
        return 1;
    }

    if (manager_loyalty < 0 || manager_loyalty > 10) {
        printf("Ошибка: лояльность должна быть от 0 до 10.\n");
        return 1;
    }

    int invitation_count = 0;
    int accepted_count = 0;
    int accepted_fee_total = 0;
    int minimum_fee = minimum_fee_for_loyalty(manager_loyalty);

    for (int attempt = 0; attempt < SIMULATION_ATTEMPTS; attempt++) {
        int social_status = rand() % 11;
        int invitation_check = rand() % 10 + 1;

        if (!got_invitation(social_status, invitation_check)) {
            continue;
        }

        invitation_count++;

        int offered_fee = 100 + (rand() % 9) * 50;
        int social_bonus = social_bonus_for_status(social_status);
        int final_fee = offered_fee + social_bonus;

        if (final_fee >= minimum_fee) {
            accepted_count++;
            accepted_fee_total += final_fee;
        }
    }

    printf("\nРезультаты %d попыток:\n", SIMULATION_ATTEMPTS);
    printf("Приглашений пришло: %d\n", invitation_count);

    if (invitation_count > 0) {
        int acceptance_percent = accepted_count * 100 / invitation_count;
        printf("Принято приглашений: %d из %d (%d%%)\n",
               accepted_count, invitation_count, acceptance_percent);
    } else {
        printf("Приглашений не было.\n");
    }

    if (accepted_count > 0) {
        int average_fee = accepted_fee_total / accepted_count;
        printf("Средний итоговый гонорар принятых выступлений: %d\n",
               average_fee);
    }

    return 0;
}
