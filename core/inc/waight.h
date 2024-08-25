#ifndef SELFMA_WEIGHT_H
#define SELFMA_WEIGHT_H

typedef void (*on_waight)();
typedef float (*on_set_waight)(float);

class Waight final {
    public:

        Waight(on_waight ow, on_set_waight osw) : _on_waight(ow), _on_set_waight(osw) { _waight = 0; };
        ~Waight() = default;
        void update() {
            _waight = _on_set_waight(_waight);
            if (_waight <= 0) {
                _on_waight();
            }
        }
    private:
        float _waight;
        on_set_waight _on_set_waight;
        on_waight _on_waight;
};

#endif // SELFMA_WIGHT_H
