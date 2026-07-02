#ifndef ZONE_HPP
    #define ZONE_HPP
    #include <string>

class Zone {
    private:
        std::string name_;
        double energyYield_;
    public:
        Zone(std::string name, double energyYield):
            name_(name), energyYield_(energyYield) {
            }
        
        const std::string& get_name(void) const
        {
            return name_;
        }
        double get_energyYield(void) const
        {
            return energyYield_;
        }
        // Modifiable par les pouvoirs divins (bénir / assécher une zone).
        void set_energyYield(double y)
        {
            energyYield_ = y < 0.0 ? 0.0 : y;
        }
};
#endif