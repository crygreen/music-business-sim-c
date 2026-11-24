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

int artist_royalty_for_week(double weekly_revenue_usd, int artist_share_percent);
int weeks_to_recoup(int remaining_usd, int weekly_royalties_usd);
double expected_revenue_from_report(
    int low_revenue_usd,
    double base_revenue_usd,
    int high_revenue_usd,
    int low_probability_percent,
    int base_probability_percent,
    int high_probability_percent
);
double expected_weeks_from_report(
    int low_weeks,
    int base_weeks,
    int high_weeks,
    int low_probability_percent,
    int base_probability_percent,
    int high_probability_percent
);

void print_revenue_scenarios(struct Track *track, int remaining_usd,
                             int artist_share_percent,
                             int low_probability_percent,
                             int base_probability_percent,
                             int high_probability_percent)
{
    int smallest_change =
        track->weeks[1].revenue_usd - track->weeks[0].revenue_usd;
    int largest_change = smallest_change;

    for (int week = 2; week < WEEKS_PER_TRACK; week++) {
        int weekly_change =
            track->weeks[week].revenue_usd -
            track->weeks[week - 1].revenue_usd;

        if (weekly_change < smallest_change) {
            smallest_change = weekly_change;
        }
        if (weekly_change > largest_change) {
            largest_change = weekly_change;
        }
    }

    int last_week_revenue = track->weeks[WEEKS_PER_TRACK - 1].revenue_usd;
    int low_scenario = last_week_revenue + smallest_change;
    int high_scenario = last_week_revenue + largest_change;

    if (low_scenario < 0) {
        low_scenario = 0;
    }
    if (high_scenario < 0) {
        high_scenario = 0;
    }

    struct TrackReport report = analyze_track(track);
    printf("Сценарии дохода на следующую неделю: слабый $%d, базовый $%.1f, сильный $%d\n",
           low_scenario,
           report.next_week_revenue_forecast,
           high_scenario);

    int low_royalties =
        artist_royalty_for_week(low_scenario, artist_share_percent);
    int base_royalties =
        artist_royalty_for_week(report.next_week_revenue_forecast,
                                artist_share_percent);
    int high_royalties =
        artist_royalty_for_week(high_scenario, artist_share_percent);

    int low_weeks = weeks_to_recoup(remaining_usd, low_royalties);
    int base_weeks = weeks_to_recoup(remaining_usd, base_royalties);
    int high_weeks = weeks_to_recoup(remaining_usd, high_royalties);

    if (low_weeks < 0 || base_weeks < 0 || high_weeks < 0) {
        printf("В одном из сценариев отчисления не позволяют оценить срок возврата.\n");
    } else {
        printf("Срок возврата по сценариям: слабый %d недель, базовый %d, сильный %d\n",
               low_weeks, base_weeks, high_weeks);
    }

    double expected_revenue_usd = expected_revenue_from_report(
        low_scenario,
        report.next_week_revenue_forecast,
        high_scenario,
        low_probability_percent,
        base_probability_percent,
        high_probability_percent
    );

    double expected_scenario_weeks = expected_weeks_from_report(
        low_weeks,
        base_weeks,
        high_weeks,
        low_probability_percent,
        base_probability_percent,
        high_probability_percent
    );

    int expected_royalties_usd =
        artist_royalty_for_week(expected_revenue_usd, artist_share_percent);
    int expected_weeks_to_recoup =
        weeks_to_recoup(remaining_usd, expected_royalties_usd);

    printf("Отчёт менеджера: вероятности слабого/базового/сильного исхода %d%%/%d%%/%d%%\n",
           low_probability_percent,
           base_probability_percent,
           high_probability_percent);
    printf("Ожидаемый доход по отчёту: $%.1f, возврат примерно за %d недель\n",
           expected_revenue_usd, expected_weeks_to_recoup);
    if (expected_scenario_weeks >= 0.0) {
        printf("Средний срок по вероятностям сценариев: %.1f недель\n",
               expected_scenario_weeks);
    } else {
        printf("Средний срок по сценариям оценить нельзя: один из сценариев не окупается.\n");
    }
}

struct RoyaltySettlement {
    int payout_usd;
    int remaining_usd;
};

struct RoyaltySettlement settle_royalties(int royalties_usd, int remaining_usd)
{
    int recouped_usd = royalties_usd;
    if (recouped_usd > remaining_usd) {
        recouped_usd = remaining_usd;
    }

    struct RoyaltySettlement result = {
        royalties_usd - recouped_usd,
        remaining_usd - recouped_usd
    };
    return result;
}

int artist_royalty_for_week(double weekly_revenue_usd, int artist_share_percent)
{
    return (int)(weekly_revenue_usd * artist_share_percent / 100.0);
}

int initial_recoupable_balance(
    int advance_usd,
    int recording_cost_usd,
    bool recording_is_recoupable,
    int marketing_cost_usd,
    bool marketing_is_recoupable
)
{
    int balance = advance_usd;

    if (recording_is_recoupable) {
        balance += recording_cost_usd;
    }

    if (marketing_is_recoupable) {
        balance += marketing_cost_usd;
    }

    return balance;
}

int revenue_needed_to_recoup(int remaining_usd, int artist_share_percent)
{
    if (artist_share_percent <= 0) {
        return -1;
    }

    return (remaining_usd * 100 + artist_share_percent - 1) /
           artist_share_percent;
}

int weeks_to_recoup(int remaining_usd, int weekly_royalties_usd)
{
    if (remaining_usd <= 0) {
        return 0;
    }
    if (weekly_royalties_usd <= 0) {
        return -1;
    }

    return (remaining_usd + weekly_royalties_usd - 1) /
           weekly_royalties_usd;
}

double expected_revenue_from_report(
    int low_revenue_usd,
    double base_revenue_usd,
    int high_revenue_usd,
    int low_probability_percent,
    int base_probability_percent,
    int high_probability_percent
)
{
    int probability_total =
        low_probability_percent +
        base_probability_percent +
        high_probability_percent;

    if (low_probability_percent < 0 ||
        base_probability_percent < 0 ||
        high_probability_percent < 0 ||
        probability_total != 100) {
        return -1.0;
    }

    return low_revenue_usd * low_probability_percent / 100.0 +
           base_revenue_usd * base_probability_percent / 100.0 +
           high_revenue_usd * high_probability_percent / 100.0;
}

double expected_weeks_from_report(
    int low_weeks,
    int base_weeks,
    int high_weeks,
    int low_probability_percent,
    int base_probability_percent,
    int high_probability_percent
)
{
    if ((low_probability_percent > 0 && low_weeks < 0) ||
        (base_probability_percent > 0 && base_weeks < 0) ||
        (high_probability_percent > 0 && high_weeks < 0)) {
        return -1.0;
    }

    return low_weeks * (low_probability_percent / 100.0) +
           base_weeks * (base_probability_percent / 100.0) +
           high_weeks * (high_probability_percent / 100.0);
}

void preview_contract(struct Track *track,
                      int low_probability_percent,
                      int base_probability_percent,
                      int high_probability_percent)
{
    const int artist_share_percent = 40;
    const int advance_usd = 500;
    struct TrackReport report = analyze_track(track);

    int remaining_usd = initial_recoupable_balance(
        advance_usd,
        track->recording_cost_usd,
        true,
        report.total_ad_spend_usd,
        true
    );

    printf("\nПример контракта для трека «%s»\n", track->title);
    printf("Доля артиста: %d%%, начальный остаток: $%d\n",
           artist_share_percent, remaining_usd);
    printf("Доход для полного возмещения: $%d\n",
           revenue_needed_to_recoup(remaining_usd, artist_share_percent));
    print_revenue_scenarios(track, remaining_usd, artist_share_percent,
                            low_probability_percent,
                            base_probability_percent,
                            high_probability_percent);

    for (int week = 0; week < WEEKS_PER_TRACK; week++) {
        int royalties_usd = artist_royalty_for_week(
            track->weeks[week].revenue_usd,
            artist_share_percent
        );
        struct RoyaltySettlement settlement =
            settle_royalties(royalties_usd, remaining_usd);

        printf("Неделя %d: отчисления $%d, выплата артисту $%d, остаток $%d\n",
               week + 1,
               royalties_usd,
               settlement.payout_usd,
               settlement.remaining_usd);

        remaining_usd = settlement.remaining_usd;
    }
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

    int low_probability_percent;
    int base_probability_percent;
    int high_probability_percent;

    printf("\nОценка менеджера. Введите вероятности слабого, базового и сильного сценариев (в сумме 100): ");
    if (scanf("%d %d %d", &low_probability_percent,
              &base_probability_percent, &high_probability_percent) != 3) {
        printf("Ошибка: введите три целых числа.\n");
        return 1;
    }

    if (low_probability_percent < 0 ||
        base_probability_percent < 0 ||
        high_probability_percent < 0 ||
        low_probability_percent + base_probability_percent +
            high_probability_percent != 100) {
        printf("Ошибка: вероятности должны быть неотрицательными и в сумме давать 100.\n");
        return 1;
    }

    preview_contract(&album.tracks[1],
                     low_probability_percent,
                     base_probability_percent,
                     high_probability_percent);

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
