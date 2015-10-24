/*-------------FDTD.cpp-------------------------------------------------------//
*
*              Finite Difference Time Domain
*
* Purpose: To replicate the results of our invisible lense raytracer with 
*          FDTD. Woo!
*
*   Notes: Most of this is coming from the following link:
*             http://www.eecs.wsu.edu/~schneidj/ufdtd/chap3.pdf
*
*-----------------------------------------------------------------------------*/

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>

static const size_t space = 200;

struct Loss{
    std::vector <double> EzH = std::vector<double>(space * space, 0), 
                         EzE = std::vector<double>(space * space, 0), 
                         HyE = std::vector<double>(space * space, 0), 
                         HyH = std::vector<double>(space * space, 0),
                         HxE = std::vector<double>(space * space, 0), 
                         HxH = std::vector<double>(space * space, 0);
};

struct Field{
    std::vector <double> Hx = std::vector<double>(space * space, 0), 
                         Hy = std::vector<double>(space * space, 0),
                         Ez = std::vector<double>(space * space, 0);
};

#define EzH(i, j) EzH[(i) + (j) *  space]
#define EzE(i, j) EzE[(i) + (j) *  space]
#define HyH(i, j) HyH[(i) + (j) *  space]
#define HyE(i, j) HyE[(i) + (j) *  space]
#define HxH(i, j) HxH[(i) + (j) *  space]
#define HxE(i, j) HxE[(i) + (j) *  space]
#define Hx(i, j) Hx[(i) + (j) *  space] 
#define Hy(i, j) Hy[(i) + (j) *  space] 
#define Ez(i, j) Ez[(i) + (j) *  space] 

void FDTD(Field EM,
          const int final_time, const double eps, const int space, Loss lass,
          std::ofstream& output);

/*----------------------------------------------------------------------------//
* MAIN
*-----------------------------------------------------------------------------*/

int main(){

    // defines output
    std::ofstream output("FDTD.dat", std::ofstream::out);

    int space = 200, final_time = 100;
    double eps = 377.0;

    Loss lass;

    // define initial E and H fields
    // std::vector<double> Ez(space, 0.0), Hy(space, 0.0);
    Field EM;

    FDTD(EM, final_time, eps, space, lass, output);

}

/*----------------------------------------------------------------------------//
* SUBROUTINES
*-----------------------------------------------------------------------------*/

// This is the function we writs the bulk of the code in
void FDTD(Field EM,
          const int final_time, const double eps, const int space, Loss lass,
          std::ofstream& output){

    // For magnetic field:
    // double offset = 0.00005;

    // for electric field:
    double offset = 0.05;
    double loss = 0.0;

    // Relative permittivity
    for (int dx = 0; dx < space; dx++){
        for (int dy = 0; dy < space; dy++){
            if (dx > 100 && dx < 150){
                lass.EzH(dx, dy) =  eps / 9.0 /(1.0 - loss);
                lass.EzE(dx, dy) = (1.0 - loss) / (1.0 + loss);
                lass.HyH(dx, dy) = (1.0 - loss) / (1.0 + loss);
                lass.HyE(dx, dy) = (1.0 / eps) / (1.0 + loss);
                lass.HxE(dx, dy) = (1.0 / eps) / (1.0 + loss);
                lass.HxH(dx, dy) = (1.0 - loss) / (1.0 + loss);
            }
            else{
                lass.EzH(dx, dy) =  eps;
                lass.EzE(dx, dy) = 1.0;
                lass.HyH(dx, dy) = 1.0;
                lass.HyE(dx, dy) = (1.0 / eps);
                lass.HxE(dx, dy) = (1.0 / eps);
                lass.HxH(dx, dy) = 1.0;
            }
        }
    }

    // Time looping
    for (int t = 0; t < final_time; t++){

        // Linking the final two elements for an ABC
        // Hy[space - 1] = Hy[space - 2];

        // update magnetic field, y direction
        for (int dx = 0; dx < space - 1; dx++){
            for (int dy = 0; dy < space; dy++){
               EM.Hy(dx,dy) = lass.HyH(dx,dy) * EM.Hy(dx,dy) 
                           + lass.HyE(dx,dy) * (EM.Ez(dx + 1,dy) 
                                                - EM.Ez(dx,dy));
            }
        }

        // update magnetic field, x direction
        for (int dx = 0; dx < space; dx++){
            for (int dy = 0; dy < space - 1; dy++){
               EM.Hx(dx,dy) = lass.HxH(dx,dy) * EM.Hx(dx, dy) 
                           + lass.HxE(dx,dy) * (EM.Ez(dx,dy + 1) 
                                                - EM.Ez(dx,dy));
            }
        }


        // Correction to the H field for the TFSF boundary
        // Hy[49] -= exp(-(t - 40.) * (t - 40.)/100.0) / eps;
        // Hy[49] -= sin((t-10.0)*0.2)*0.0005;


        // Linking the first two elements in the electric field
        EM.Ez[0] = EM.Ez[1];
        // Ez[space - 1] = Ez[space - 2];

        // update electric field
        for (int dx = 1; dx < space - 1; dx++){
            for (int dy = 1; dy < space - 1; dy++){
               EM.Ez(dx,dy) = lass.EzE(dx,dy) * EM.Ez(dx,dy)
                           + lass.EzH[dx] * (EM.Hy(dx, dy)
                                             - EM.Hy(dx - 1, dy)
                                             - EM.Hx(dx,dy)
                                             - EM.Hx(dx, dy - 1));
            }
        }

        // set src for next step
        EM.Ez[50] += exp(-((t + 1 - 40.) * (t + 1 - 40.))/100.0);
        // EM.Ez[0] = 0;
        
        // Ez[50] += sin((t - 10.0 + 1)*0.2)*0.0005;
/*
        if (t > 0){
            Ez[50] = sin(0.1 * t) / (0.1 * t);
        }
        else{
            Ez[50] = 1;
        }
*/
        if (t % 50 == 0){
            for (int dx = 0; dx < space; dx = dx + 5){
                for (int dy = 0; dy < space; dy = dy + 5){
                    output << t << '\t' << dx <<'\t' << dy << '\t'
                           << EM.Hx(dx, dy) << '\n';
                    //output << Ez(dx,dy) + (t * offset) << '\n';
                    //output << Hy[dx] + (t * offset) << '\n';
                }
            }

            output << '\n' << '\n';
        }

    }
}
