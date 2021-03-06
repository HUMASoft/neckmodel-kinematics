

#include <iostream>
#include "Cia402device.h"
#include "CiA301CommPort.h"
#include "SocketCanPort.h"
#include "mainlib.h"
#include "math.h"
#include "ToolsFControl.h"

#include "SerialArduino.h"

#include "fcontrol.h"


int main ()
{
        //--sensors--
        SerialArduino tilt;
        float incSensor,oriSensor;

        double dts=0.01; //0.01
    ofstream graph("graph.csv",std::ofstream::out);

//    long incl_value=20;

    //--Controllers--
    //fpi w=25 pm=70 //kept from last experiments.
    vector<double> npi ={0.2916 ,  -5.3981 ,   0.2415  ,  5.5243};
    vector<double> dpi ={0.5882 , -0.9921 , -0.5804 , 1.0000};
    SystemBlock internal1(npi,dpi);
    SystemBlock internal3(npi,dpi);
    SystemBlock internal2(npi,dpi);

//    string method("pi");
//    PIDBlock internal1(1,10,0,dts);
//    PIDBlock internal2(1,10,0,dts);
//    PIDBlock internal3(1,10,0,dts);

//    string method("2isomw10p100");

    //Fraccionario Externo IJARS
//    vector<double> npd ={-0.1572, 1.2479, 0.1469, -4.3659, 3.1780};
//    vector<double> dpd ={-0.0571, 0.4310, 0.1080, -1.4818, 1.0000};
//    SystemBlock external1(npd,dpd,1);
//    SystemBlock external2(npd,dpd,1);
//    SystemBlock external3(npd,dpd,1);

//    string method("w10p60pid"); // entero
//     PIDBlock external1(2,1,0.1,dts);
//     PIDBlock external2(2,1,0.1,dts);
//     PIDBlock external3(2,1,0.1,dts);

    // entero IJARS
    string method("w10p60pid");
     PIDBlock external1(3.2158394,29.966573,0.0000061,dts);
     PIDBlock external2(3.2158394,29.966573,0.0000061,dts);
     PIDBlock external3(3.2158394,29.966573,0.0000061,dts);

//////     fraccionario (kp,kf,exp)
//    string method("w10p60pid");
//     FPDBlock external1(2.988612,22.744199,-0.9041765,dts);
//     FPDBlock external2(2.988612,22.744199,-0.9041765,dts);
//     FPDBlock external3(2.988612,22.744199,-0.9041765,dts);

//    // entero isatrans
//    string method("w10p60pid");
//    PIDBlock external1(1.95,7.76,0,dts);
//    PIDBlock external2(1.95,7.76,0,dts);
//    PIDBlock external3(1.95,7.76,0,dts);

//     //     fraccionario (kp,kf,exp) isatrans
//     string method("w10p60pid");
//     FPDBlock external1(1.76,4.7872,-0.81,dts);
//     FPDBlock external2(1.76,4.7872,-0.81,dts);
//     FPDBlock external3(1.76,4.7872,-0.81,dts);


    //--savefiles--
//    string folder("/home/humasoft/Escritorio/");
//    ofstream targets (folder+method+".targets.csv");
//    ofstream responses (folder+method+".responses.csv");
//    ofstream controls (folder+method+".controls.csv");
//    ofstream graph("graph.csv",std::ofstream::out);


    //--Can port communications--
    SocketCanPort pm1("can1");
    CiA402Device m1 (1, &pm1);
    SocketCanPort pm2("can1");
    CiA402Device m2 (2, &pm2);
    SocketCanPort pm3("can1");
    CiA402Device m3 (3, &pm3);


    //--Neck Kinematics--
    TableKinematics a("../neck-control/arco107.csv");
    vector<double> lengths(3);


    //vars
    double ep1,ev1,cs1,tcs1;
    double ep2,ev2,cs2,tcs2;
    double ep3,ev3,cs3,tcs3;
    double posan1, posan2, posan3;


//    Motor setup
    m1.Reset();
    m1.SwitchOn();

    m2.Reset();
    m2.SwitchOn();

    m3.Reset();
    m3.SwitchOn();


    m3.Setup_Torque_Mode();
    m1.Setup_Torque_Mode();
    m2.Setup_Torque_Mode();

    long orient=1;
    long incli=1;
    float lg0=0.107;
    sleep(4);


    ToolsFControl tools;
    tools.SetSamplingTime(dts);

    //-----------Soft Start

//    for (int i=1; i<incl_value; i++){
//    incli ++;
    orient=90;
    double wptime=2;
    vector<long> inclis={10,20,1};
//    incli=incl_value; //comentar esta linea para el for anterior


//    for (double t=0;t<200;t++){
    //           incSensor = tilt.ReadInclination();
    //            oriSensor = tilt.ReadOrientation();

    for (int wp=0; wp<inclis.size(); wp++)
    {
        incli=inclis[wp];

        for (double t=0;t<wptime;t+=dts){ //<0.05

            tilt.readSensor(incSensor,oriSensor);


            cout  << "incli " << incli << ",  orient " << orient  << endl;
            cout << "incli_sen: " << incSensor << " , orient_sen: " << oriSensor <<  endl;
            a.GetIK(incli,orient,lengths);
            posan1=(lg0-lengths[0])*180/(0.01*M_PI);
            posan2=(lg0-lengths[1])*180/(0.01*M_PI);
            posan3=(lg0-lengths[2])*180/(0.01*M_PI);
            cout << "TARGET: , " << posan1  << " , " << posan2 << " , " << posan3 << endl;


            ep1=posan1-m1.GetPosition();
            cs1=ep1 > external1;
            ev1= cs1-m1.GetVelocity();
            tcs1=(ev1 > internal1);
            m1.SetTorque(tcs1);

            ep2=posan2-m2.GetPosition();
            cs2=ep2 > external2;
            ev2= cs2-m2.GetVelocity();
            tcs2= (ev2 > internal2);
            m2.SetTorque(tcs2);

            ep3=posan3-m3.GetPosition();
            cs3=ep3 > external3;
            ev3= cs3-m3.GetVelocity();
            tcs3= (ev3 > internal3);
            m3.SetTorque(tcs3);


            cout << "ACTUAL: , " << m1.GetPosition() << " , " << m2.GetPosition() <<  " , " << m3.GetPosition() <<endl<<endl;
            graph << t << " , " << posan1 << " , " << m1.GetPosition() << " , " << posan2 << " , " << m2.GetPosition()  << " , " << posan3 <<  " , " << m3.GetPosition()  << " , " << incli << " , " << incSensor   << " , " << orient   << " , " << oriSensor << " , " << ep1 << " , " << cs1 << " , " << ev1 << " , " << tcs1 << " , " << ep2 << " , " << cs2 << " , " << ev2 << " , " << tcs2 << " , " << ep3 << " , " << cs3 << " , " << ev3 << " , " << tcs3 <<endl;

            tools.WaitSamplingTime();
        } //for (double t=0;t<5;t+=dts){ //<0.05


        // } //for (double t=0;t<200;t++)

    }  //    for (int wp=0; wp<inclis.size(); wp++)

    cout << "FIN" << endl;
    sleep(3);


    m1.SetTorque(0);
    m2.SetTorque(0);
    m3.SetTorque(0);

    sleep(1);

//    targets.close();
//    controls.close();
//    responses.close();
    return 0;
}
