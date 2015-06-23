
void MY_SOLVE() {
   //
   // <INTEGRATION>
   //
   M.setZero();
   K.setZero();
   v.setZero();
   f.setZero();
   Eigen::Matrix3d I = Eigen::Matrix3d::Identity();

   // Fill in mass matrix
   for (int i = 0; i < particles.size(); i++)
      if (!particles[i]->fixed) {
         int ndx = particles[i]->i;
         M.block<3,3>(ndx,ndx) = particles[i]->m * I;
      }

   // Fill in velocity vector
   for (int i = 0; i < particles.size(); i++)
      if (!particles[i]->fixed) {
         int ndx = particles[i]->i;
         v.segment<3>(ndx) = particles[i]->v;
      }

   // Fill in force vector
   for (int i = 0; i < particles.size(); i++)
      if (!particles[i]->fixed) {
         int ndx = particles[i]->i;
         f.segment<3>(ndx) = particles[i]->m * grav;
      }

   // Add in spring forces
   for (int i = 0; i < springsCollisions.size(); i++) {
      Spring * s = springsCollisions[i];
      Particle * p0 = s->p0;
      Particle * p1 = s->p1;
      Eigen::Vector3d DX = (p1->x - p0->x);
      double l = DX.norm();
      Eigen::Vector3d springF = s->K * (l - s->L) * DX / l;

      if (!p0->fixed)
         f.segment<3>(p0->i) += springF;
      if (!p1->fixed)
         f.segment<3>(p1->i) += -springF;
   }

   // Fill in stiffness matrix
   for (int i = 0; i < springsCollisions.size(); i++) {
      Spring * s = springsCollisions[i];
      Particle * p0 = s->p0;
      Particle * p1 = s->p1;
      Eigen::Matrix<double, 3, 1> DX = (p1->x - p0->x);
      Eigen::Matrix<double, 1, 3> DX_t = DX.transpose();
      double l = DX.norm();
      double lRatio = (l - s->L) / l;
      double DXt_DX_f = (DX_t * DX);
      Eigen::Matrix3d DXt_DX = DXt_DX_f * I;
      Eigen::Matrix3d DX_DXt = (DX * DX_t);

      Eigen::Matrix3d Ks = (-s->K / (l*l)) * ((lRatio * DXt_DX) + (1 - lRatio) * DX_DXt);
      if (!p0->fixed && !p1->fixed) {
         K.block<3,3>(p0->i, p0->i) += Ks;
         K.block<3,3>(p0->i, p1->i) += -1.0*Ks;
         K.block<3,3>(p1->i, p0->i) += -1.0*Ks;
         K.block<3,3>(p1->i, p1->i) += Ks;
      } else if (!p0->fixed)
         K.block<3,3>(p0->i, p0->i) += Ks;
      else if (!p1->fixed)
         K.block<3,3>(p1->i, p1->i) += Ks;
   }

   // Solve the equation
   Eigen::MatrixXd A = M - (h*h)*K;
   Eigen::MatrixXd b = (M * v) + (h * f);
   Eigen::VectorXd Vnew = A.ldlt().solve(b);

   // Update velocities
   for (int i = 0; i < particles.size(); i++)
      if(!(particles[i]->fixed))
         particles[i]->v = Vnew.segment<3>(particles[i]->i);

   // Update positions
   for (int i = 0; i < particles.size(); i++)
      particles[i]->x += h * particles[i]->v;

   // Update position and normal buffers
   updatePosNor();

   // Delete collision springs
   while(!collisions.empty()) {
      delete collisions.back();
      collisions.pop_back();
   }
}
