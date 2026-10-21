#ifndef SPHERES_HPP
#define SPHERES_HPP

#include "common.hh"

struct PI_GAME
{
    PI_GAME();

    bool app_event(const PI_EVENT& event) noexcept;
    bool app_update() noexcept;

    bool ok() const noexcept { return hscene; }

    PI_HWORLDENV_RAII hworldenv;
    PI_HSCENE_RAII hscene;

    PI_HMESH_RAII hsphere;
    PI_HMESH_RAII hcube;

    std::vector<SPHERE> spheres;

    CUBE ground;
};

#endif
