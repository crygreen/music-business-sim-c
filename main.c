#include <stdbool.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

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

int artist_royalty_for_revenue(double revenue_usd, int artist_share_percent)
{
    return (int)(revenue_usd * artist_share_percent / 100.0);
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


#define CAMPAIGN_ROUNDS 5
#define WEEKS_PER_YEAR 52

enum Scenario { LOW, BASE, HIGH, SCENARIO_COUNT };
enum Strategy { BALANCED = 1, ADVANCE_FIRST, DIVERT_RIVAL };

static const char *const scenario_names[] = {"слабый", "базовый", "сильный"};
static const char *const rank_names[] = {"Новичок", "Растущий артист", "Хедлайнер"};

struct Probabilities {
    int low;
    int base;
    int high;
};

struct Artist {
    int cash;
    int followers;
    int reputation;
    int reliability;
    int trust_in_manager;
    int catalog_revenue;
    int released_tracks;
};

struct Manager {
    int salary;
    int salary_earned;
    int commission_earned;
    int relationship;
    int director_trust;
    int allocated;
    int returned;
    int forecasts_checked;
    double brier_total;
};

struct Label {
    int cash;
    int rival_trust;
};

struct Contract {
    int advance;
    int artist_share;
    int years;
    int advertising;
    int producer_fee;
    int lawyer_fee;
    int commission_percent;
    bool artist_counsel;
};

struct RoundEvents {
    int market_shift;
    int social_boost;
    int artist_pressure;
    int concert_fee;
    bool health_setback;
    bool concert_cancelled;
    bool rival_slot;
    bool offer_delayed;
};

struct DealResult {
    int gross;
    int artist_royalties_paid;
    int label_receipts;
    int unrecouped;
};

static int clamp(int value, int minimum, int maximum)
{
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

static int minimum(int first, int second)
{
    return first < second ? first : second;
}

static int random_below(int limit)
{
    const unsigned range = (unsigned)RAND_MAX + 1U;
    const unsigned end = range - range % (unsigned)limit;
    unsigned draw;

    do {
        draw = (unsigned)rand();
    } while (draw >= end);

    return (int)(draw % (unsigned)limit);
}

static bool read_strategy(enum Strategy *strategy)
{
    char line[64];

    while (fgets(line, sizeof line, stdin) != NULL) {
        char *end;
        long number = strtol(line, &end, 10);
        if (strchr(line, '\n') == NULL && !feof(stdin)) {
            int next;
            do {
                next = getchar();
            } while (next != '\n' && next != EOF);
            puts("Введите один номер от 1 до 3.");
            continue;
        }
        if (end != line && number >= BALANCED && number <= DIVERT_RIVAL &&
            end[strspn(end, " \t\r\n")] == '\0') {
            *strategy = (enum Strategy)number;
            return true;
        }
        puts("Введите один номер от 1 до 3.");
    }
    return false;
}

static enum Scenario choose_scenario(struct Probabilities odds)
{
    int draw = random_below(100) + 1;

    if (draw <= odds.low) {
        return LOW;
    }
    if (draw <= odds.low + odds.base) {
        return BASE;
    }
    return HIGH;
}

static double brier_score(struct Probabilities report, enum Scenario actual)
{
    double low_error = report.low / 100.0 - (actual == LOW);
    double base_error = report.base / 100.0 - (actual == BASE);
    double high_error = report.high / 100.0 - (actual == HIGH);

    return low_error * low_error + base_error * base_error +
           high_error * high_error;
}

static int artist_rank(const struct Artist *artist)
{
    if (artist->reputation >= 70) {
        return 2;
    }
    if (artist->reputation >= 40) {
        return 1;
    }
    return 0;
}

static struct RoundEvents draw_events(const struct Artist *artist)
{
    struct RoundEvents events = {0};
    int market = random_below(5);
    int social = random_below(5);

    if (market == 0) {
        events.market_shift = -12;
    } else if (market == 1) {
        events.market_shift = 10;
    }
    if (social == 0) {
        events.social_boost = 4;
    } else if (social == 1) {
        events.social_boost = 12;
    }
    int pressure = random_below(5);
    if (pressure == 0) {
        events.artist_pressure = 500;
    } else if (pressure == 1) {
        events.artist_pressure = 1100;
    }

    events.health_setback = random_below(100) >= artist->reliability;
    events.concert_cancelled = random_below(5) == 0 || events.health_setback;
    events.concert_fee = 200 + 50 * random_below(5);
    events.rival_slot = random_below(3) != 0;
    events.offer_delayed = random_below(6) == 0;
    return events;
}

static struct Contract make_offer(enum Strategy strategy, int rank)
{
    if (strategy == ADVANCE_FIRST) {
        return (struct Contract){1300, 28 + 3 * rank, 3, 450, 350, 120, 10, false};
    }
    if (strategy == DIVERT_RIVAL) {
        return (struct Contract){350, 57 + 3 * rank, 1, 160, 200, 120, 10, true};
    }
    return (struct Contract){500, 64 + 3 * rank, 1, 260, 300, 120, 10, true};
}

static struct Probabilities release_odds(const struct Artist *artist,
                                         struct Contract offer, int shift)
{
    int audience_effect = clamp(artist->followers / 1800, 0, 12);
    int advertising_effect = 22 * offer.advertising / (offer.advertising + 300);
    int producer_effect = offer.producer_fee / 80;
    int change = audience_effect + artist_rank(artist) * 4 +
                 advertising_effect + producer_effect + shift;
    int low = clamp(40 - change, 5, 75);
    int high = clamp(15 + change, 5, 75);

    return (struct Probabilities){low, 100 - low - high, high};
}

static struct Probabilities make_report(struct Probabilities estimate,
                                        enum Strategy strategy)
{
    int shift = 0;
    if (strategy == ADVANCE_FIRST) {
        shift = 20;
    } else if (strategy == DIVERT_RIVAL) {
        shift = 10;
    }
    shift = minimum(shift, estimate.low + estimate.base);
    int from_low = minimum(shift, estimate.low);

    return (struct Probabilities){
        estimate.low - from_low,
        estimate.base - (shift - from_low),
        estimate.high + shift
    };
}

static void revenue_scenarios(struct Track *track, struct Contract offer,
                              int weekly[SCENARIO_COUNT])
{
    struct TrackReport report = analyze_track(track);
    int smallest_change = track->weeks[1].revenue_usd - track->weeks[0].revenue_usd;
    int largest_change = smallest_change;

    for (int week = 2; week < WEEKS_PER_TRACK; week++) {
        int change = track->weeks[week].revenue_usd -
                     track->weeks[week - 1].revenue_usd;
        if (change < smallest_change) {
            smallest_change = change;
        }
        if (change > largest_change) {
            largest_change = change;
        }
    }
    int latest = track->weeks[WEEKS_PER_TRACK - 1].revenue_usd;
    int promotion = offer.advertising / 28 + offer.producer_fee / 80;

    weekly[LOW] = clamp(latest + smallest_change, 0, 100000) / 2;
    weekly[BASE] = (int)(report.next_week_revenue_forecast + 0.5) + promotion;
    weekly[HIGH] = 2 * (latest + largest_change + promotion);
}

static int commission_for_offer(struct Contract offer)
{
    return offer.advance * offer.commission_percent / 100;
}

static int deal_cost(const struct Track *track, struct Contract offer)
{
    return offer.advance + track->recording_cost_usd + offer.advertising +
           offer.producer_fee + offer.lawyer_fee + commission_for_offer(offer);
}

static int recoup_balance(const struct Track *track, struct Contract offer)
{
    return initial_recoupable_balance(
        offer.advance, track->recording_cost_usd, true,
        offer.advertising, !offer.artist_counsel
    );
}

static struct DealResult settle_release(int weekly_revenue,
                                        const struct Track *track,
                                        struct Contract offer)
{
    int weeks = offer.years * WEEKS_PER_YEAR;
    int gross = weekly_revenue * weeks;
    int royalties = artist_royalty_for_revenue(gross, offer.artist_share);
    int balance = recoup_balance(track, offer);
    struct RoyaltySettlement settlement = settle_royalties(royalties, balance);

    return (struct DealResult){
        gross, settlement.payout_usd, gross - settlement.payout_usd,
        settlement.remaining_usd
    };
}

static double weighted_value(const int values[SCENARIO_COUNT],
                             struct Probabilities odds)
{
    return values[LOW] * (odds.low / 100.0) +
           values[BASE] * (odds.base / 100.0) +
           values[HIGH] * (odds.high / 100.0);
}

static void print_recoup_forecast(int balance, struct Contract offer,
                                 const int weekly[SCENARIO_COUNT],
                                 struct Probabilities report)
{
    int weeks[SCENARIO_COUNT];
    const int weights[] = {report.low, report.base, report.high};

    for (int scenario = LOW; scenario < SCENARIO_COUNT; scenario++) {
        int cents_per_week = weekly[scenario] * offer.artist_share;
        if (cents_per_week == 0 && weights[scenario] > 0) {
            puts("Есть шанс, что отчислений для возмещения не будет.");
            return;
        }
        weeks[scenario] = cents_per_week == 0 ? 0 :
                          (100 * balance + cents_per_week - 1) / cents_per_week;
        if (weeks[scenario] > offer.years * WEEKS_PER_YEAR && weights[scenario] > 0) {
            puts("Есть шанс, что расходы не возместятся до конца прав.");
            return;
        }
    }

    printf("Средний прогноз срока возмещения по сценариям: %.1f нед.\n",
           weighted_value(weeks, report));
}

static int director_limit(const struct Label *label, const struct Artist *artist,
                          const struct Manager *manager)
{
    int artist_limit = 2400 + artist->reputation * 12 +
                       minimum(artist->catalog_revenue / 30, 800);
    int return_percent = 0;
    if (manager->allocated > 0) {
        return_percent = clamp(100 * manager->returned / manager->allocated - 100,
                               -100, 100);
    }
    int manager_limit = 1750 + 13 * manager->director_trust +
                        45 * manager->relationship + 4 * return_percent;

    return minimum(label->cash, minimum(artist_limit, manager_limit));
}

static bool artist_accepts_offer(const struct Artist *artist, struct Contract offer,
                                 int pressure, double projected_income,
                                 double independent_income)
{
    int shortage = clamp(pressure - artist->cash, 0, 1100);
    int urgency_value = minimum(shortage, offer.advance) * 8;
    int rights_cost = (offer.years - 1) * (120 + artist->reputation * 3);
    int trust_value = (artist->trust_in_manager - 50) * 8;
    return offer.advance + projected_income + urgency_value +
           trust_value >= independent_income + rights_cost;
}

static void handle_rival(struct Label *label, struct Manager *manager,
                         enum Strategy strategy, bool available)
{
    if (!available) {
        return;
    }
    if (strategy == DIVERT_RIVAL) {
        manager->relationship = clamp(manager->relationship + 2, 0, 10);
        puts("Соперника отвлекла другая сделка. Вы укрепили связь с директором.");
        return;
    }
    if (label->cash < 500 || label->rival_trust < 30) {
        return;
    }
    label->cash -= 500;
    int receipts = random_below(100) < 55 ? 750 : 300;
    label->cash += receipts;
    label->rival_trust = clamp(label->rival_trust +
                              (receipts > 500 ? 4 : -5), 0, 100);
    printf("Сделка соперника: вложено $500, вернулось $%d. Доверие к нему: %d.\n",
           receipts, label->rival_trust);
}

static void handle_concert(struct Artist *artist, struct Manager *manager,
                           struct RoundEvents events)
{
    int status = clamp(artist->reputation / 10, 0, 10);
    if (!got_invitation(status, random_below(10) + 1)) {
        puts("Концертного предложения в этом раунде нет.");
        return;
    }
    if (events.concert_cancelled) {
        int cancellation_cost = minimum(artist->cash, 50);
        artist->cash -= cancellation_cost;
        artist->reputation = clamp(artist->reputation - 3, 0, 100);
        printf("Концерт сорвался. Потери артиста $%d, репутация снизилась.\n",
               cancellation_cost);
        return;
    }
    int fee = events.concert_fee + social_bonus_for_status(status);
    if (fee >= minimum_fee_for_loyalty(artist->trust_in_manager / 10)) {
        int commission = fee / 10;
        manager->commission_earned += commission;
        artist->cash += fee - commission;
        artist->reputation = clamp(artist->reputation + 2, 0, 100);
        printf("Концерт состоялся. Артист получил $%d, комиссия $%d.\n",
               fee - commission, commission);
    }
}

static void settle_turn(struct Artist *artist, struct Manager *manager,
                        struct Label *label, struct Track *track,
                        struct RoundEvents events, enum Strategy strategy,
                        int *album_revenue)
{
    struct Contract offer = make_offer(strategy, artist_rank(artist));
    int weekly[SCENARIO_COUNT];
    revenue_scenarios(track, offer, weekly);
    struct Probabilities estimate = release_odds(
        artist, offer, events.market_shift
    );
    struct Probabilities report = make_report(estimate, strategy);

    int funding = deal_cost(track, offer);
    int artist_payout[SCENARIO_COUNT];
    int label_receipts[SCENARIO_COUNT];
    int independent[SCENARIO_COUNT];
    struct Contract independent_offer = {.years = offer.years};
    int own_weekly[SCENARIO_COUNT];
    revenue_scenarios(track, independent_offer, own_weekly);

    for (int scenario = LOW; scenario < SCENARIO_COUNT; scenario++) {
        struct DealResult result = settle_release(weekly[scenario], track, offer);
        artist_payout[scenario] = result.artist_royalties_paid;
        label_receipts[scenario] = result.label_receipts;
        // The artist keeps 70 percent after independent distribution costs.
        independent[scenario] = own_weekly[scenario] * offer.years *
                                WEEKS_PER_YEAR * 70 / 100 - track->recording_cost_usd;
    }
    int limit = director_limit(label, artist, manager);
    double label_forecast = weighted_value(label_receipts, report) - funding;
    double fair_forecast = weighted_value(label_receipts, estimate) - funding;
    double expected_payout = weighted_value(artist_payout, estimate);
    struct Probabilities independent_odds =
        release_odds(artist, independent_offer, events.market_shift);
    double expected_independent = weighted_value(independent, independent_odds);

    printf("Прогноз менеджера: %d%% / %d%% / %d%%; оценка модели: %d%% / %d%% / %d%%.\n",
           report.low, report.base, report.high,
           estimate.low, estimate.base, estimate.high);
    printf("Доход за неделю по сценариям: $%d / $%d / $%d. Средний по отчёту $%.2f.\n",
           weekly[LOW], weekly[BASE], weekly[HIGH], weighted_value(weekly, report));
    printf("Сделка: аванс $%d, доля артиста %d%%, срок %d г., реклама $%d.\n",
           offer.advance, offer.artist_share, offer.years, offer.advertising);
    printf("Директор: лимит $%d, запрос $%d. Прибыль по отчёту $%.0f, по модели $%.0f.\n",
           limit, funding, label_forecast, fair_forecast);

    print_recoup_forecast(recoup_balance(track, offer), offer, weekly, report);

    int exaggeration = report.high - estimate.high;
    if (exaggeration > 0 && random_below(100) < 25 + 2 * exaggeration) {
        manager->director_trust = clamp(manager->director_trust - 12, 0, 100);
        label_forecast = fair_forecast;
        limit = director_limit(label, artist, manager);
        printf("Аудит обнаружил приукрашенный прогноз. Новый лимит $%d.\n", limit);
    }
    if (events.offer_delayed) {
        puts("Площадка для премьеры сорвала сроки. Сделка отложена.");
        return;
    }
    double required_profit = funding * 0.25 * offer.years +
                             (100 - manager->director_trust) * 4;
    printf("Минимальная прогнозная прибыль для директора: $%.0f.\n",
           required_profit);
    if (funding > limit || label_forecast < required_profit) {
        puts("Директор отклонил сделку. Средства остались у лейбла.");
        return;
    }
    printf("Артист сравнивает контракт ($%.0f с авансом) и самостоятельный выпуск ($%.0f).\n",
           expected_payout + offer.advance, expected_independent);
    if (!artist_accepts_offer(artist, offer, events.artist_pressure,
                              expected_payout, expected_independent)) {
        puts("Артист отказался от контракта.");
        return;
    }
    if (offer.years > 1) {
        artist->trust_in_manager = clamp(artist->trust_in_manager - 9, 0, 100);
    } else {
        artist->trust_in_manager = clamp(artist->trust_in_manager + 4, 0, 100);
    }

    label->cash -= funding;
    manager->allocated += funding;
    manager->commission_earned += commission_for_offer(offer);
    int event_shift = events.market_shift + events.social_boost;
    if (events.health_setback) {
        event_shift -= 15;
    }
    enum Scenario actual = choose_scenario(release_odds(artist, offer, event_shift));
    struct DealResult result = settle_release(weekly[actual], track, offer);
    double error = brier_score(report, actual);

    manager->forecasts_checked++;
    manager->brier_total += error;
    double average_error = manager->brier_total / manager->forecasts_checked;
    if (manager->forecasts_checked >= 2) {
        if (average_error > 0.75) {
            manager->director_trust = clamp(manager->director_trust - 6, 0, 100);
        } else if (average_error < 0.35) {
            manager->director_trust = clamp(manager->director_trust + 4, 0, 100);
        }
    }

    label->cash += result.label_receipts;
    manager->returned += result.label_receipts;
    artist->cash += offer.advance + result.artist_royalties_paid;
    artist->catalog_revenue += result.gross;
    artist->released_tracks++;
    *album_revenue += result.gross;
    artist->followers += weekly[actual] * 5;
    const int reputation_change[] = {-4, 6, 14};
    artist->reputation = clamp(artist->reputation + reputation_change[actual], 0, 100);
    if (actual == HIGH && offer.years > 1) {
        int independent_actual = independent[actual];
        int signed_income = offer.advance + result.artist_royalties_paid;
        if (independent_actual > signed_income) {
            printf("Трек взлетел. При таком же исходе самостоятельный выпуск принёс бы на $%d больше.\n",
                   independent_actual - signed_income);
        }
    }
    printf("Исход: %s. Выручка $%d, артисту $%d сверх аванса, лейблу $%d.\n",
           scenario_names[actual], result.gross, result.artist_royalties_paid,
           result.label_receipts);
    printf("Продюсер получил $%d. Юрист получил $%d: %s.\n",
           offer.producer_fee, offer.lawyer_fee,
           offer.artist_counsel ? "защитил отчисления от списания рекламы" :
                                  "оформил возврат расходов на рекламу");
    printf("К концу срока прав не возмещено $%d. Ошибка Брайера %.2f, средняя %.2f.\n",
           result.unrecouped, error, average_error);
}



static int run_campaign(struct Album *albums, bool demo)
{
    struct Artist artist = {210, 4200, 30, 88, 54, 0, 0};
    struct Manager manager = {900, 0, 0, 5, 55, 0, 0, 0, 0.0};
    struct Label label = {15000, 50};
    const enum Strategy demo_strategies[CAMPAIGN_ROUNDS] = {
        BALANCED, ADVANCE_FIRST, DIVERT_RIVAL, ADVANCE_FIRST, BALANCED
    };
    int album_revenue[2] = {0};

    puts("СИМУЛЯТОР МУЗЫКАЛЬНОГО БИЗНЕСА");
    printf("Касса лейбла $%d. Зарплата менеджера $%d за раунд.\n",
           label.cash, manager.salary);

    for (int turn = 0; turn < CAMPAIGN_ROUNDS; turn++) {
        if (label.cash < manager.salary) {
            puts("Лейбл не может оплатить следующий раунд. Кампания окончена.");
            break;
        }
        int album_index = turn / TRACKS_PER_ALBUM;
        struct Album *album = &albums[album_index];
        struct Track *track = &album->tracks[turn % TRACKS_PER_ALBUM];

        if (turn % TRACKS_PER_ALBUM == 0) {
            print_album_report(album);
        }
        printf("\n--- Раунд %d/%d. «%s» ---\n",
               turn + 1, CAMPAIGN_ROUNDS, track->title);
        printf("%s. Подписчиков %d, репутация %d, доверие артиста %d.\n",
               rank_names[artist_rank(&artist)], artist.followers,
               artist.reputation, artist.trust_in_manager);
        printf("Выручка каталога $%d, из них $%d у текущего альбома. Касса артиста $%d.\n",
               artist.catalog_revenue, album_revenue[album_index], artist.cash);

        struct RoundEvents events = draw_events(&artist);
        if (events.market_shift < 0) {
            puts("Рынок просел. Слабый сценарий стал вероятнее.");
        } else if (events.market_shift > 0) {
            puts("Стриминги дали жанру больше места. Шансы релиза выросли.");
        }
        if (events.artist_pressure == 500) {
            puts("Близкие давят на артиста из-за срочных расходов.");
        } else if (events.artist_pressure > 0) {
            puts("Партнёр артиста настаивает на крупном авансе из-за семейных расходов.");
        }

        enum Strategy strategy;
        if (demo) {
            strategy = demo_strategies[turn];
            printf("Демо: стратегия %d.\n", strategy);
        } else {
            puts("1 — сбалансированный контракт и честный отчёт");
            puts("2 — крупный аванс, длинные права и оптимистичный отчёт");
            puts("3 — дешёвая сделка и попытка отвлечь конкурента");
            if (!read_strategy(&strategy)) {
                puts("\nВвод завершён.");
                break;
            }
        }

        label.cash -= manager.salary;
        manager.salary_earned += manager.salary;
        handle_rival(&label, &manager, strategy, events.rival_slot);
        if (events.health_setback) {
            puts("Проблемы из-за рискованной привычки сорвали репетицию. Шансы релиза снизились.");
            artist.reliability = clamp(artist.reliability - 4, 70, 98);
        } else {
            artist.reliability = clamp(artist.reliability + 2, 70, 98);
        }
        if (events.social_boost > 0) {
            int new_followers = events.social_boost * 55;
            artist.followers += new_followers;
            printf("Пост в соцсетях привлёк %d подписчиков и дал дополнительный охват.\n",
                   new_followers);
        }
        settle_turn(&artist, &manager, &label, track, events, strategy,
                    &album_revenue[album_index]);
        handle_concert(&artist, &manager, events);

        int paid_expenses = minimum(events.artist_pressure, artist.cash);
        if (paid_expenses > 0) {
            artist.cash -= paid_expenses;
            printf("Артист направил $%d на срочные расходы.\n", paid_expenses);
        }
        printf("Касса лейбла $%d, артиста $%d. Доверие директора %d, связи %d/10.\n",
               label.cash, artist.cash, manager.director_trust, manager.relationship);
    }

    puts("\nИТОГ КАМПАНИИ");
    printf("Подписано релизов: %d. Артист: $%d, %d подписчиков, статус «%s».\n",
           artist.released_tracks, artist.cash, artist.followers,
           rank_names[artist_rank(&artist)]);
    for (int album = 0; album < 2; album++) {
        printf("Альбом «%s»: выручка по сделкам $%d.\n",
               albums[album].title, album_revenue[album]);
    }
    printf("Лейбл: $%d. Менеджер: зарплата $%d, комиссии $%d.\n",
           label.cash, manager.salary_earned, manager.commission_earned);
    if (manager.forecasts_checked > 0) {
        printf("Проверено прогнозов: %d. Средняя ошибка Брайера: %.2f.\n",
               manager.forecasts_checked,
               manager.brier_total / manager.forecasts_checked);
    } else {
        puts("Проверка прогноза требует хотя бы одной подписанной сделки.");
    }
    return 0;
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    bool demo = argc >= 2 && strcmp(argv[1], "--demo") == 0;
    if (argc > 3 || (argc > 1 && !demo)) {
        fprintf(stderr, "Использование: %s [--demo [seed]]\n", argv[0]);
        return 1;
    }
    unsigned seed = demo ? 34U : (unsigned)time(NULL);
    if (argc == 3) {
        char *end;
        errno = 0;
        unsigned long parsed = strtoul(argv[2], &end, 10);
        if (argv[2][0] < '0' || argv[2][0] > '9' || end == argv[2] ||
            *end != '\0' || errno == ERANGE || parsed > UINT_MAX) {
            fputs("Seed должен быть целым неотрицательным числом.\n", stderr);
            return 1;
        }
        seed = (unsigned)parsed;
    }
    srand(seed);

    struct Album albums[] = {
        {
            "После полуночи", 3,
            {
                {"Сигнал", 400, {{10000, 15000, 750, 300, 45, 100},
                                 {13000, 21000, 1200, 500, 63, 80},
                                 {16000, 27000, 1800, 750, 81, 60}}},
                {"После полуночи", 300, {{8000, 12000, 400, 140, 36, 50},
                                         {8500, 13000, 420, 150, 39, 40},
                                         {8200, 12500, 410, 130, 37, 30}}},
                {"Статика", 350, {{5000, 7000, 210, 80, 21, 30},
                                  {9000, 13000, 650, 250, 39, 50},
                                  {18000, 28000, 2000, 950, 84, 120}}}
            }
        },
        {
            "Город не спит", 2,
            {
                {"Неон", 340, {{13500, 18000, 780, 250, 54, 90},
                                {15800, 21500, 900, 390, 65, 100},
                                {16800, 24000, 1200, 440, 72, 110}}},
                {"Последний эфир", 420, {{8000, 14000, 450, 150, 42, 75},
                                         {12000, 19000, 760, 300, 57, 90},
                                         {17000, 26000, 1300, 520, 78, 130}}}
            }
        }
    };
    return run_campaign(albums, demo);
}
