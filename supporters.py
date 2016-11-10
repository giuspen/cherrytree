#!/usr/bin/env python2
# -*- coding: UTF-8 -*-

import re

SUPPORTERS="""
<h2>Friends of giuspen.com (donated €3137,53)</h2>
• Miguel Latorre (Spain) donated €240
• DistroWatch.com (International) donated €200
• Alan M. (United States) donated €100
• Mario (Austria) donated €100
• 1001bricks (France) donated €80 [custom keyboard shortcuts]
• Michael Moyer (United States) donated €70
• Charles Leis (United States) donated €65
• Rene Gommes (Italy) donated €55
• Samir Derzic (Germany) donated €55
• Stuart Krivis (United States) donated €55
• Barbara Buri (Switzerland) donated €50
• Ger de Gooijer (Netherlands) donated €50 [mark node in the tree with bold or color]
• Manuel Pacheco (Spain) donated €50
• Rudy Witt (Canada) donated €50
• Ted (Germany) donated €45 [same tree visible twice for drag n drop]
• Andrew Marquis (United Kingdom) donated €40
• Jack Downes (United States) donated €40
• Sylvain Pasche (Switzerland) donated €40
• Thomas Ludwig (Germany) donated €40
• Juan Rivera (Spain) donated €35
• Giulio Achilli (Italy) donated €30
• Jan Axelsson (Sweden) donated €30 [nodes icons or colors and nodes separator]
• Jesse Sloane (South Korea) donated €30
• Josh Hanson (United States) donated €30 [sub item pressing the TAB key in lists]
• MC Merchants (United States) donated €30
• Miguel Gestal (Spain) donated €30
• Ravinderpal Vaid (United States) donated €30
• Steven Rockefeller (United States) donated €30
• Sebastian Prodan (Germany) donated €26
• Andy Lavarre (United States) donated €25
• Bruce Ashford (United Kingdom) donated €25
• Édipo Gonçalves (Brazil) donated €25
• James Lin (United States) donated €25
• Miguel Vilar (Switzerland) donated €25
• Soren ONeill (Denmark) donated €25
• Steve Davis (United States) donated €25
• Jaroslav Polacek (Czech Republic) donated €22,22
• Andrew Golovanov (United States) donated €20
• Anthony Miller (United Kingdom) donated €20
• David Condray (United States) donated €20
• Emmanuel Chantreau (France) donated €20
• Francesco Zaniol (Australia) donated €20
• Frank Dehmel (Germany) donated €20
• Gaurav Pal (United Kingdom) donated €20
• Geoffrey Munn (United Kingdom) donated €20
• Jaroslav Svobboda (Czech Republic) donated €20
• Joerg Materna (Germany) donated €20
• John Parkin (United Kingdom) donated €20
• José Martínez (Spain) donated €20
• Lee Rottler (United States) donated €20
• Michael Jabbour (Austria) donated €20
• Michael Matney (United States) donated €20
• Randall Raziano (United States) donated €20
• Roger Rowles (Australia) donated €20
• Rudy Witt (Canada) donated €20
• Timothy DeCant (United States) donated €20
• Wolfgang See (Germany) donated €20
• Zigurds Gavars (Latvia) donated €20 [better table cells support]
• Christopher Brazill (United States) donated €15
• Hans van Meteren (Netherlands) donated €15
• Istvan Cebrian (Portugal) donated €15
• John Duchek (United States) donated €15
• John Grant (United Kingdom) donated €15
• Joshua Chalifour (Canada) donated €15
• Kees Wiebering (Germany) donated €15
• Lukas Golombek (Germany) donated €15
• Marwan Nader (Canada) donated €15
• Patricia Bonardi (Netherlands) donated €15
• Duncan M.K. (United Kingdom) donated €12,5
• Bennett Z. Kobb (United States) donated €12,39
• Dandi Soft (Italy) donated €12
• Administrateur (France) donated €10
• Balint Fekete (Hungary) donated €10
• Benoît D’Angelo (France) donated €10
• Chipmunk Software &amp; Systems (United States) donated €10
• Christoph Rottleb (Germany) donated €10
• Claus Karstensen (Denmark) donated €10
• David Allinson (United Kingdom) donated €10
• David Butcher (United Kingdom) donated €10
• Dennis Roberts (United States) donated €10
• Derek Perry (Canada) donated €10
• Eric Jourdan (France) donated €10
• Glenna Drake (United States) donated €10
• Gregory Bruccoleri (United States) donated €10
• iCore effective GmbH (Germany) donated €10
• Jesus Arocho (United States) donated €10
• Kåre Jensen (Denmark) donated €10
• Ken Dangerfield (Canada) donated €10
• Lachlan Brown (Australia) donated €10
• Laura Haglund (United States) donated €10
• Leonardo Gonçalves (Brazil) donated €10
• Marilena Marrone (Italy) donated €10
• Mika Kujanpää (Finland) donated €10
• Mustafa Kabakcioglu (Turkey) donated €10
• Oscar (Mexico) donated €10
• Passionate Awakenings (United States) donated €10
• Paul Saletan (United States) donated €10
• Petr Bartel (Czech Republic) donated €10
• Ranjit Singh (Germany) donated €10
• Robert Harris (United States) donated €10
• Ronald Cameron (United States) donated €10
• Rudy Richardson (United States) donated €10
• Silton Tennis (United States) donated €10
• Stefano Landi (Canada) donated €10
• Stridor Media (Germany) donated €10
• Tai Wei Feng (Australia) donated €10
• Thomas Gruschwitz (Germany) donated €10
• Thomas Locquet (France) donated €10
• Thomas Polomski (Germany) donated €10
• ZebraMap (United States) donated €10
• James Skahan (United States) donated €9 [shortcut to leave the codebox]
• David Severn (United States) donated €8,82
• Daniel Siefert (Germany) donated €8
• Richard Trefz (United States) donated €8
• Winfred McCarty (United States) donated €7,66
• Angus Rose (United Kingdom) donated €7
• Gerard Dubrulle (France) donated €7
• Glen Garfein (United States) donated €7
• Tara Stewart (United States) donated €6
• Thomas Bondois (France) donated €6
• Nick Cross (United Kingdom) donated €5,80
• Alexander Eckert (Germany) donated €5
• Alexander Lackner (Germany) donated €5
• Benjamin Westwood (United Kingdom) donated €5
• Daniel Elias (Brazil) donated €5
• Gerben Tijkken (Netherlands) donated €5
• Gloria See (United States) donated €5
• Ivo Grigull (Germany) donated €5
• Karsten Kulach (Germany) donated €5
• Marian Hanzel (Slovakia) donated €5
• Mario Tosques (Italy) donated €5
• Marius Van der Merwe (Australia) donated €5
• Massimo Beltramin (Italy) donated €5
• Max Greisen (Belgium) donated €5
• Maxime Lahaye (United States) donated €5
• Michael Schönwälder (Germany) donated €5
• Olivier Le Moal (France) donated €5
• Open Source Solutions (Bulgaria) donated €5
• Paul Robinson (United Kingdom) donated €5
• Piotr Swadzba (Ireland) donated €5
• Sol Hübner (Germany) donated €5
• Tal Liron (United States) donated €5
• The Dick Turpin Road Show (United Kingdom) donated €5
• Лев Выскубов (Russia) donated €5
• Jarius Elliott (United States) donated €4,74
• Zachary Peterson (United States) donated €3,64
• Frank Maniscalco (Canada) donated €3,50
• Hugo McPhee (Australia) donated €3,26
• Tomáš Chalúpek (Czech Republic) donated €3
• Frank Lazar (Germany) donated €2
• Виталий Волков (Russia) donated €2
"""

sum_donations = float(0)
countries_dict = {}
for supporter_line in SUPPORTERS.split("\n"):
    match = re.search(r"\(([^\)]+)\) donated €(\d+,\d+|\d+)", supporter_line, re.UNICODE)
    if match:
        curr_country = match.group(1)
        curr_donation = float(match.group(2).replace(",","."))
        sum_donations += curr_donation
        if not curr_country in countries_dict.keys():
            countries_dict[curr_country] = float(0)
        countries_dict[curr_country] += curr_donation
    else:
        print supporter_line
print sum_donations
while countries_dict:
    curr_max_key = None
    for curr_key, curr_val in countries_dict.iteritems():
        if not curr_max_key:
            curr_max_key = curr_key
        elif curr_val > countries_dict[curr_max_key]:
            curr_max_key = curr_key
    print curr_max_key, countries_dict[curr_max_key]
    del countries_dict[curr_max_key]
