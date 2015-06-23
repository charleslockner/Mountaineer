
typedef void (*dydt_func)(double t, double y[], double ydot[]);
void implicit_euler_ode(double y0[], double yend[], int len, double t0, double t1, dydt_func dydt);

struct RigidBody {
   /* Constant quantities */
   float              mass;      /* mass M */
   Eigen::Matrix3f    Ibody,     /* inertia tensor (in body space) */
   Eigen::Matrix3f    Ibodyinv;  /* inverse of the inertia tensor (in body space) */

   /* State variables */
   Eigen::Vector3f    x;         /* x(t) = translation (in world) */
   Eigen::Quaternionf q;         /* q(t) = rotation (in world) */
   Eigen::Vector3f    P;         /* P(t) = linear momentum */
   Eigen::Vector3f    L;         /* L(t) = angular momentum */

   /* Derived quantities (auxiliary variables) */
   Eigen::Matrix3f    Iinv;      /* I−1(t) = inverse of inertia tensor (in world space) */
   Eigen::Matrix3f    R;         /* R(t) = rotation matrix */
   Eigen::Vector3f    v;         /* v(t) = linear velocity */
   Eigen::Vector3f    omega;     /* ω(t) = axis of rotation who's magnitude is angular velocity */

   /* Computed quantities */
   Eigen::Vector3f    force;     /* F(t) = total force acting on the body */
   Eigen::Vector3f    torque;    /* τ(t) = total torque acting on the body */
};

#define STATE_SIZE 13
#define NBODIES 1

RigidBody Bodies[NBODIES];

/* Copy the state information into an array */
void State_to_Array(RigidBody *rb, double * stateArr) {
   *stateArr++ = rb->x[0]; /* x component of position */
   *stateArr++ = rb->x[1]; /* etc. */
   *stateArr++ = rb->x[2];

   /*
      Assume that a quaternion is represented in
      terms of elements '0' for the real part,
      and '1', '2', and '3' for the vector part.
   */
   *stateArr++ = rb->q(0);
   *stateArr++ = rb->q(1);
   *stateArr++ = rb->q(2);
   *stateArr++ = rb->q(3);

   *stateArr++ = rb->P[0];
   *stateArr++ = rb->P[1];
   *stateArr++ = rb->P[2];

   *stateArr++ = rb->L[0];
   *stateArr++ = rb->L[1];
   *stateArr++ = rb->L[2];
}

/* Copy information from an array into the state variables */
void Array_to_State(RigidBody *rb, double *stateArr) {
   rb->x[0] = *stateArr++;
   rb->x[1] = *stateArr++;
   rb->x[2] = *stateArr++;

   rb->q(0) = *stateArr++;
   rb->q(1) = *stateArr++;
   rb->q(2) = *stateArr++;
   rb->q(3) = *stateArr++;

   rb->P[0] = *stateArr++;
   rb->P[1] = *stateArr++;
   rb->P[2] = *stateArr++;

   rb->L[0] = *stateArr++;
   rb->L[1] = *stateArr++;
   rb->L[2] = *stateArr++;
}

void Array_to_Bodies(double y[]) {
   for(int i = 0; i < NBODIES; i++)
      Array_to_State(& Bodies[i], & y[i * STATE_SIZE]);
}

void Bodies_to_Array(double y[]) {
   for(int i = 0; i < NBODIES; i++)
      State_to_Array(& Bodies[i], & y[i * STATE_SIZE]);
}

void ddt_State_to_Array(RigidBody * rb, double * ydot) {
   /* copy d/dt x(t) = v(t) into ydot */
   *ydot++ = rb->v[0];
   *ydot++ = rb->v[1];
   *ydot++ = rb->v[2];

   Eigen::Quaternionf qdot = 0.5 * (Mmath::AngleAxisQuat(0, rb->omega) * rb->q);
   *ydot++ = qdot(0);
   *ydot++ = qdot(1);
   *ydot++ = qdot(2);
   *ydot++ = qdot(3);

   *ydot++ = rb->force[0]; /* d/dt P(t) = F(t) */
   *ydot++ = rb->force[1];
   *ydot++ = rb->force[2];

   *ydot++ = rb->torque[0]; /* d/dt L(t) = τ(t) */
   *ydot++ = rb->torque[1];
   *ydot++ = rb->torque[2];
}

void dydt(double t, double y[], double ydot[]) {
   /* put data in y[ ] into Bodies[ ] */
   Array_to_Bodies(y);
   for (int i = 0; i < NBODIES; i++) {
      Compute_Force_and_Torque(t, & Bodies[i]);
      ddt_State_to_Array(& Bodies[i], & ydot[i * STATE_SIZE]);
   }
}

void implicit_euler_ode(double y0[], double yend[], int len, double t0, double t1, dydt_func dydt) {
   double h = t1 - t0;
   Eigen::Matrix3d I = Eigen::Matrix3d::Identity();

   double * ydotM = (double *)malloc(len * sizeof(double)); // should be a matrix?
   Eigen::MatrixXd ydelta;
   dydt(t, y, ydot);

   Eigen::MatrixXd A = ((1.0/h)*I - ydotM);
   Eigen::MatrixXd B =
   ydelta = A.ldlt().solve(b);
   // yend = y0 + ydelta;
}

void RunSimulation() {
   double y0[STATE_SIZE * NBODIES];
   double yfinal[STATE_SIZE * NBODIES];

   InitStates();
   Bodies_to_Array(yfinal);

   for (double t = 0; t < 10.0; t += 1.0/30.0) {
      /* copy yfinal back to y0 */
      for (int i = 0; i < STATE_SIZE * NBODIES; i++)
         y0[i] = yfinal[i];

      implicit_euler_ode(y0, yfinal, STATE_SIZE * NBODIES, t, t+1.0/30.0, dydt);

      /* copy d/dt Y(t + 1/30 ) into state variables */
      Array_to_Bodies(yfinal);
      DisplayBodies();
   }
}

