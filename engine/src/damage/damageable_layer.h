#ifndef DAMAGEABLELAYER_H
#define DAMAGEABLELAYER_H

#include <vector>

#include "damageable_facet.h"
#include "core_vector.h"

/**
 * @brief The DamageableLayer class represents an object to be damaged.
 * This can be shields, armor, hull or subsystem.
 * A layer can be regenerative (e.g. shields) or not (everything else).
 */
class DamageableLayer
{
public:
    bool core_layer;    // Damage to the core layer has a chance of also
                        // damaging internal components such as propulsion.

private:
    unsigned int number_of_facets;    // How many facets. e.g. dual shield (front and rear).
    std::vector<DamageableFacet> facets;    // The facets container

    //static float damage_component_chance = 0.03;
public:
    DamageableLayer(int number_of_facets, std::vector<DamageableFacet>& facets, bool core_layer):
        number_of_facets(number_of_facets),
        facets(facets),
        core_layer(core_layer) {}

    DamageableLayer():
        core_layer(false),
        number_of_facets(1) {
        facets.push_back(DamageableFacet());
    }

    void DealDamage( const CoreVector &attack_vector, Damage &damage );
    void Disable();
    void Enable();
    const DamageableFacet& GetFacet(const CoreVector& attack_vector);
};

#endif // DAMAGEABLELAYER_H
