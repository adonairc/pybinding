#pragma once
#include "Model.hpp"

#include "utils/Chrono.hpp"
#include "utils/Log.hpp"

#include "support/dense.hpp"
#include "support/thread.hpp"

#include <memory>

namespace tbm {

class Hamiltonian;
template <class scalar_t> class HamiltonianT;

/**
 Abstract base class for Green's function strategy.
 */
class GreensStrategy {
public:
    virtual ~GreensStrategy() = default;

    /// Try to set the Hamiltonian
    /// @return false if the given Hamiltonian is the wrong scalar type for this GreensStrategy
    virtual bool set_hamiltonian(const std::shared_ptr<const Hamiltonian>& hamiltonian) = 0;
    /// Return the Green's function at (i,j) for the given energy range
    virtual ArrayXcf calculate(int i, int j, ArrayXf energy, float broadening) = 0;
    /// Get some information about what happened during the last calculation
    virtual std::string report(bool shortform) const = 0;
};


/**
 Abstract base with type specialization.
 */
template<class scalar_t>
class GreensStrategyT : public GreensStrategy {
public:
    virtual ~GreensStrategyT() { Log::d("~GreensStrategy<" + num::scalar_name<scalar_t>() + ">()"); }

    virtual bool set_hamiltonian(const std::shared_ptr<const Hamiltonian>& ham) final {
        // check if it's compatible
        if (auto cast_ham = std::dynamic_pointer_cast<const HamiltonianT<scalar_t>>(ham)) {
            if (hamiltonian != cast_ham) {
                hamiltonian = cast_ham;
                hamiltonian_changed();
            }
            return true;
        }
        // failed -> wrong scalar_type
        return false;
    }

protected:
    /// post-processing that may be defined by derived classes
    virtual void hamiltonian_changed() {};

protected:
    std::shared_ptr<const HamiltonianT<scalar_t>> hamiltonian; ///< the Hamiltonian to solve
};


/**
 Green's function calculation interface.
 Internally it uses a GreensStrategy with the scalar of the given Hamiltonian.
 Derived classes must implement create_strategy_for(hamiltonian).
 */
class Greens {
public:
    virtual ~Greens() = default;

public:
    void set_model(Model const&);
    Model const& get_model() const { return model; }
    std::shared_ptr<System const> system() const { return model.system(); }

    /// Get some information about what happened during the last calculation
    std::string report(bool shortform) const {
        return strategy ? strategy->report(shortform) + " " + calculation_timer.str() : "";
    }

    ArrayXcf calc_greens(int i, int j, ArrayXf energy, float broadening);
    ArrayXf calc_ldos(ArrayXf energy, float broadening,
                      Cartesian position, sub_id sublattice = -1);
    Deferred<ArrayXf> deferred_ldos(ArrayXf energy, float broadening,
                                    Cartesian position, sub_id sublattice = -1);

protected:
    /// Create a new Green's strategy object for the given Hamiltonian
    virtual std::unique_ptr<GreensStrategy>
        create_strategy_for(const std::shared_ptr<const Hamiltonian>&) const = 0;

protected:
    Model model;
    std::unique_ptr<GreensStrategy> strategy;
    Chrono calculation_timer; ///< last calculation time
};

} // namespace tbm