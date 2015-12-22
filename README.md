This is a port of the Arduino Tide Calculator to the Particle Photon.
See https://github.com/millerlp/Tide_calculator

I modified hist Generate_new_site_libraries/tied_harmonics_library_generator.R (see my photon_tied_harmonics_library_generator.R)

Choose the desired station name from: http://www.flaterco.com/xtide/locations.html
Be sure to choose a 'Ref' station. Note that 'Sub' stations are not supported at this time

Run the R script from a Cygwin shell. This will generate folder with your .h and .cpp files
     export PATH=$PATH:/cygdrive/c/Program\ Files/R/R-3.2.3/bin
     RScript photon_tide_harmonics_library_generator.R

