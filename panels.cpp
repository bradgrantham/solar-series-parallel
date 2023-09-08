#include <vector>
#include <cstdlib>
#include <cstdio>
#include <numeric>
#include <algorithm>

struct source
{
    virtual float current() const = 0;
    virtual float voltage() const = 0;
};

struct panel : public source
{
    float c;
    float v;
    panel(float c, float v) : c(c), v(v) {}
    virtual float current() const override { return c; }
    virtual float voltage() const override { return v; }
};

struct parallel : public source
{
    std::vector<const source*> sources;
    parallel(const std::vector<const source*>& sources) : sources(sources) {};
    virtual float current() const override
    {
        auto sum = [](float v, const source* s){return v + s->current();};
        return std::accumulate(sources.begin(), sources.end(), 0.0, sum);
    }
    virtual float voltage() const override
    {
        auto min = [](float v, const source* s){return std::min(v, s->voltage());};
        return std::accumulate(sources.begin(), sources.end(), 1e6, min);
    }
};

struct serial : public source
{
    std::vector<const source*> sources;
    serial(const std::vector<const source*>& sources) : sources(sources) {};
    virtual float voltage() const override
    {
        auto sum = [](float v, const source* s){return v + s->voltage();};
        return std::accumulate(sources.begin(), sources.end(), 0.0, sum);
    }
    virtual float current() const override
    {
        auto min = [](float v, const source* s){return std::min(v, s->current());};
        return std::accumulate(sources.begin(), sources.end(), 1e6, min);
    }
};

float power(const source& s)
{
    return s.current() * s.voltage();
}

int main(int argc, char **argv)
{
    panel kc50t(3.11, 17.4);
    panel sun100(5.44, 18.4);
    panel newpowa220(12.6, 17.52);

    serial _4s_kc50t({&kc50t, &kc50t, &kc50t, &kc50t});
    parallel _4p_kc50t({&kc50t, &kc50t, &kc50t, &kc50t});
    parallel _2p_sun100({&sun100, &sun100});

    serial _4s_newpowa220({&newpowa220, &newpowa220, &newpowa220, &newpowa220});

    printf("one kc50t = %f watts\n", power(kc50t));
    printf("4P kc50t = %f watts\n", power(_4s_kc50t));
    printf("2P sun100 = %f watts\n", power(_2p_sun100));
    printf("4S kc50t = %f watts\n", power(_4p_kc50t));
    printf("4S NewPowa 220 = %f watts\n", power(_4s_newpowa220));

    serial newpowa_and_kc50ts({&_4p_kc50t, &_4s_newpowa220});
    printf("4S newpowas and 4P KC50T string = %f watts\n", power(newpowa_and_kc50ts));

    serial newpowa_and_sun100s({&_2p_sun100, &_4s_newpowa220});
    printf("4S newpowas and 2P SUN100 string = %f watts\n", power(newpowa_and_sun100s));

    // serial _2s_sun100_kc50t({&kc50t, &sun100});
    // printf("serial sun100 and kc50t = %f volts, %f watts\n", _2s_sun100_kc50t.voltage(), power(_2s_sun100_kc50t));
    // 38V but 111W, low efficiency

    // parallel _2p_sun100_kc50t({&kc50t, &sun100});
    // printf("parallel sun100 and kc50t = %f volts, %f watts\n", _2p_sun100_kc50t.voltage(), power(_2p_sun100_kc50t));
    // like 148W but 17V

    parallel _2p_kc50t({&kc50t, &kc50t});
    serial _2s_1_sun100_2p_kc50t({&sun100, &_2p_kc50t});
    serial _2s_2s_1_sun100_2p_kc50t({&_2s_1_sun100_2p_kc50t, &_2s_1_sun100_2p_kc50t});
    printf("2x (1 sun100 and 2p kc50t) = %f volts, %f watts\n", _2s_2s_1_sun100_2p_kc50t.voltage(), power(_2s_2s_1_sun100_2p_kc50t));

}
