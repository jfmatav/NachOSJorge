int main(){
	int id1, id2;

	Write("Exec: Prueba de Exec y Join", 27, 1);
	id1 = Exec("../test/exec1");
        id2=Join( id1 );
	Write("Exec: Fin de Prueba ...", 23, 1);
        Exit(id2);

	return 0;
}
