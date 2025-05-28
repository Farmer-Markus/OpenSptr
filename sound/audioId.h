#pragma once

/*enum cutSceneSound {
    zelda appears 6
    zelda soundtrack 7
    first miniser(lock putzen nicht nötig...) 8
    schienen weg, gleich delok gegen ferro 22
    zelda aufgeregt/panik(schnell beeilen körper zurück holden ...) 35
    phantom about to kill link 38
    zelda is phantom 39
    zusammen als phantom auf in den kampf 40
    ...
    Delok besiegt(kurz vor ende und er haut ab) 88
    zelda happ (nochmal hören) 101

    
};*/

enum cutSceneSound {
    ZELDA_FIRST_APPEAR = 6,
    ZELDA_SOUNDTRACK = 7,
    MINISTER_FIRST_EVIL = 8, // Minister sagt "geh deine lock putzen oder so, allerdings "musik" ist das bald ganz egal"
    DELOG_PREPARING_AGAINST_FERRO = 22, // Schienen weg, Ferro beschützt Zelda
    ZELDA_PANIC = 35, // Zelda (in turm der götter) aufgeregt: schnell zurück holen(nach "IIIIIH der hat meinen körper)") kommt immer näher zu link"du musst helfen körper zu holen, ich werde warten, das war immer die aufgabe einer prinzessin..."
    PHANTOM_ABOUT_KILL_LINK = 38,
    ZELDA_IS_INSIDE_PHANTOM = 39,
    ZELDA_READY_TO_FIGHT = 40, // Phantom Zelda mit link in den kampf(nachdem zelda in phantom gegangen ist)
    LINK_GOT_LOKOMO_SWORT = 103 // Oder rekruten uniform :/ ?
};
