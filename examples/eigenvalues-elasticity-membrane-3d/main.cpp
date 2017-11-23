#include "mesh.h"
#include "field.h"
#include "expression.h"
#include "formulation.h"
#include "vec.h"
#include "petsc.h"
#include "wallclock.h"
#include "mat.h"
#include "resolution.h"


using namespace mathop;

void sparselizard(void)
{	
    // The domain regions as defined in 'circle.geo':
    int vol = 1, sur = 2, top = 3;
    
    // The mesh can be curved!
    mesh mymesh("circle.msh");
    
    // Nodal shape functions 'h1' with 3 components.
    // Field u is the membrane deflection.
    field u("h1xyz");

    // Use interpolation order 3 on 'vol', the whole domain:
    u.setorder(vol, 3);
    
    // Clamp on surface 'sur' (i.e. 0 valued-Dirichlet conditions):
    u.setconstraint(sur);
  
    // E is Young's modulus. nu is Poisson's ratio. rho is the volumic mass.
    parameter E, nu, rho;
    E|vol = 150e9; nu|vol = 0.3, rho|vol = 2330;
  
    formulation elasticity;

    // The linear elasticity formulation is classical and thus predefined:
    elasticity += integral(vol, predefinedelasticity(u, E, nu));
    // Add the inertia terms:
    elasticity += integral(vol, -dtdt(dof(u))*tf(u));

    elasticity.generate();
    
    // Get the stiffness and mass matrix:
    mat K = elasticity.K();
    mat M = elasticity.M();

    // Remove the rows and columns corresponding to the 0 constraints:
    K.removeconstraints();
    M.removeconstraints();
    
    // Create the object to solve the generalised eigenvalue problem K*x = lambda*M*x :
    eigenvalue eig(K, M);
    
    // Compute the 10 eigenvalues closest to the target magnitude 0.0 (i.e. the 10 first ones):
    eig.compute(10, 0.0);
    
    // Print the eigenvalues, i.e. the square of the modes frequencies:
    eig.print();
    
    // The eigenvectors are real thus we only need the real part:
    std::vector<vec> myeigenvectors = eig.geteigenvectorrealpart();
    
    // Loop on all eigenvectors found:
    for (int i = 0; i < myeigenvectors.size(); i++)
    {
    	// Transfer the data from the ith eigenvector to field u:
        u.getdata(top, myeigenvectors[i]);
        // Write the deflection on the top surface of the membrane with an order 3 interpolation.
        u.write(top, "u"+std::to_string(i)+".pos", 3);
    }
}

int main(void)
{	
    SlepcInitialize(0,{},0,0);

    sparselizard();

    SlepcFinalize();

    return 0;
}








