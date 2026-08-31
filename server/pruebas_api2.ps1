$ErrorActionPreference = 'Continue'
$base = 'http://localhost:8080'
$SEP = [string][char]31

$script:pasadas = 0
$script:fallidas = 0
$script:fallos = @()
$script:seccion = ''

function Seccion($n) { $script:seccion = $n; Write-Host ""; Write-Host "=== $n ===" }
function Chk($nombre, $cond, $detalle) {
  if ($cond) { $script:pasadas++ }
  else {
    $script:fallidas++
    $script:fallos += "[$script:seccion] $nombre :: $detalle"
    Write-Host ("  FALLO  " + $nombre + "  ->  " + $detalle) -ForegroundColor Red
  }
}
function Enc($s) { return [System.Uri]::EscapeDataString([string]$s) }

function Pedir($metodo, $ruta, $cuerpo) {
  try {
    if ($metodo -eq 'GET') { $r = Invoke-WebRequest -Uri ($base + $ruta) -UseBasicParsing -TimeoutSec 20 }
    else { $r = Invoke-WebRequest -Uri ($base + $ruta) -Method Post -Body $cuerpo -ContentType 'application/x-www-form-urlencoded' -UseBasicParsing -TimeoutSec 20 }
    $j = $null; try { $j = $r.Content | ConvertFrom-Json } catch {}
    return @{ status = [int]$r.StatusCode; body = $j; raw = $r.Content; muerto = $false }
  } catch {
    $resp = $_.Exception.Response
    if ($null -eq $resp) { return @{ status = 0; body = $null; raw = $_.Exception.Message; muerto = $true } }
    $texto = ''
    try { $sr = New-Object System.IO.StreamReader($resp.GetResponseStream()); $texto = $sr.ReadToEnd() } catch {}
    $j = $null; if ($texto) { try { $j = $texto | ConvertFrom-Json } catch {} }
    return @{ status = [int]$resp.StatusCode; body = $j; raw = $texto; muerto = $false }
  }
}
function Post($ruta, $cuerpo) { return Pedir 'POST' $ruta $cuerpo }
function Get_($ruta) { return Pedir 'GET' $ruta $null }

function Crear($nombre, $estados, $simbolos, $inicial, $finales, $transiciones, $guardar) {
  $b = 'nombre=' + (Enc $nombre)
  foreach ($e in $estados)      { $b += '&estado='     + (Enc $e) }
  foreach ($s in $simbolos)     { $b += '&simbolo='    + (Enc $s) }
  if ($null -ne $inicial)       { $b += '&inicial='    + (Enc $inicial) }
  foreach ($f in $finales)      { $b += '&final='      + (Enc $f) }
  foreach ($t in $transiciones) { $b += '&transicion=' + (Enc $t) }
  return Post '/api/automatas' ($b + '&guardar=' + $guardar)
}
function Tr($o, $s, $d) { return $o + $SEP + $s + $SEP + $d }
function ProbarSimple($id, $cadena) { return Post '/api/probar-cadena' ("modo=simple&id=$id&cadena=" + (Enc $cadena)) }
function Vivo() { $r = Get_ '/api/automatas'; return (-not $r.muerto) -and ($r.status -eq 200) }

# =====================================================================
Seccion 'N. BARRIDO COMPLETO DE ASCII IMPRIMIBLE COMO SIMBOLO'
# =====================================================================
# cada char de 33 a 126 como unico simbolo del alfabeto. todos entran al
# alfabeto: el filtro ya no esta en la insercion. solo el guion tiene que
# terminar invalidando el automata, por reservado.
$reservadosEsperados = @('-')
$sorpresas = @()
for ($code = 33; $code -le 126; $code++) {
  $ch = [string][char]$code
  $r = Crear ("Ascii$code") @('q0') @($ch) 'q0' @('q0') @((Tr 'q0' $ch 'q0')) 0
  $entro = ($null -ne $r.body) -and ($r.body.automata.alfabeto.Count -eq 1)
  $deberiaValer = -not ($reservadosEsperados -contains $ch)
  $ok = $entro -and ($r.body.valido -eq $deberiaValer)
  Chk ("N ascii $code '$ch'") $ok "entro=$entro valido=$($r.body.valido) esperaba valido=$deberiaValer alfabeto=[$($r.body.automata.alfabeto -join '')]"
  if (-not $ok) { $sorpresas += "$code '$ch'" }
}
Write-Host "  simbolos con comportamiento inesperado: $(if ($sorpresas.Count -eq 0) { 'ninguno' } else { $sorpresas -join ', ' })"

# blancos: entran al alfabeto, pero el validador los nombra y los rechaza
foreach ($par in @(@{n='espacio';c=' ';t='espacio en blanco'}, @{n='tab';c=[string][char]9;t='tabulacion'}, @{n='nueva linea';c=[string][char]10;t='salto de linea'}, @{n='retorno';c=[string][char]13;t='retorno de carro'})) {
  $r = Crear ('Blanco' + $par.n) @('q0') @($par.c, 'a') 'q0' @('q0') @((Tr 'q0' 'a' 'q0')) 0
  Chk ("N blanco entra al alfabeto: " + $par.n) ($r.body.automata.alfabeto.Count -eq 2) "alfabeto count=$($r.body.automata.alfabeto.Count)"
  Chk ("N blanco invalida: " + $par.n) ($r.body.valido -eq $false) "valido=$($r.body.valido)"
  Chk ("N blanco nombrado en el error: " + $par.n) (($r.body.errores -join ' ') -like "*$($par.t)*") "errores=$($r.body.errores -join '; ')"
}

Chk 'N servidor vivo' (Vivo) 'murio en el barrido ascii'

# =====================================================================
Seccion 'O. ALFABETOS GRANDES Y ESTADOS NUMERICOS'
# =====================================================================

# 26 letras minusculas
$letras = @(); for ($i = 97; $i -le 122; $i++) { $letras += [string][char]$i }
$trans = @(); foreach ($l in $letras) { $trans += (Tr 'q0' $l 'q0') }
$O1 = Crear 'Abecedario' @('q0') $letras 'q0' @('q0') $trans 1
Chk 'O1 alfabeto de 26 letras' ($O1.body.automata.alfabeto.Count -eq 26) "alfabeto=$($O1.body.automata.alfabeto.Count)"
Chk 'O1 valido' ($O1.body.valido -eq $true) "errores=$($O1.body.errores -join '; ')"
$idO1 = $O1.body.id
$r = ProbarSimple $idO1 'zyxwvutsrq'
Chk 'O1 acepta cadena de 10 letras' ($r.body.recorridos[0].acepta -eq $true) "acepta=$($r.body.recorridos[0].acepta)"

# 10 digitos
$digitos = @(); for ($i = 0; $i -le 9; $i++) { $digitos += [string]$i }
$trans = @(); foreach ($d in $digitos) { $trans += (Tr 'd0' $d 'd0') }
$O2 = Crear 'Digitos' @('d0') $digitos 'd0' @('d0') $trans 1
Chk 'O2 alfabeto de 10 digitos' ($O2.body.automata.alfabeto.Count -eq 10) "alfabeto=$($O2.body.automata.alfabeto -join '')"
Chk 'O2 valido' ($O2.body.valido -eq $true) "errores=$($O2.body.errores -join '; ')"
$r = ProbarSimple $O2.body.id '9876543210'
Chk 'O2 acepta 9876543210' ($r.body.recorridos[0].acepta -eq $true) "acepta=$($r.body.recorridos[0].acepta)"

# 62 alfanumericos
$alnum = @()
for ($i = 48; $i -le 57; $i++) { $alnum += [string][char]$i }
for ($i = 65; $i -le 90; $i++) { $alnum += [string][char]$i }
for ($i = 97; $i -le 122; $i++) { $alnum += [string][char]$i }
$trans = @(); foreach ($c in $alnum) { $trans += (Tr 'g0' $c 'g0') }
$O3 = Crear 'Alfanumerico62' @('g0') $alnum 'g0' @('g0') $trans 1
Chk 'O3 alfabeto de 62 simbolos' ($O3.body.automata.alfabeto.Count -eq 62) "alfabeto=$($O3.body.automata.alfabeto.Count)"
Chk 'O3 valido' ($O3.body.valido -eq $true) "errores=$($O3.body.errores -join '; ')"
$idO3 = $O3.body.id
$r = ProbarSimple $idO3 'aZ0zA9'
Chk 'O3 acepta mezcla' ($r.body.recorridos[0].acepta -eq $true) "acepta=$($r.body.recorridos[0].acepta)"

# estados con nombres numericos
$O4 = Crear 'EstadosNumericos' @('0','1','00','-1','1.5') @('a') '0' @('1') @((Tr '0' 'a' '1'),(Tr '1' 'a' '00'),(Tr '00' 'a' '-1'),(Tr '-1' 'a' '1.5'),(Tr '1.5' 'a' '0')) 1
Chk 'O4 nombres numericos: 5 estados' ($O4.body.automata.estados.Count -eq 5) "estados=$($O4.body.automata.estados -join ',')"
Chk 'O4 valido' ($O4.body.valido -eq $true) "errores=$($O4.body.errores -join '; ')"
Chk 'O4 estado "0" distinto de "00"' (($O4.body.automata.estados -contains '0') -and ($O4.body.automata.estados -contains '00')) "estados=$($O4.body.automata.estados -join ',')"
$r = ProbarSimple $O4.body.id 'a'
Chk 'O4 acepta a' ($r.body.recorridos[0].acepta -eq $true) "acepta=$($r.body.recorridos[0].acepta)"

# el simbolo pipe ahora tiene que funcionar de verdad
$O5 = Crear 'SimboloPipe' @('q0','q1') @('|','a') 'q0' @('q1') @((Tr 'q0' '|' 'q1'),(Tr 'q0' 'a' 'q0'),(Tr 'q1' '|' 'q1'),(Tr 'q1' 'a' 'q0')) 1
Chk 'O5 pipe como simbolo aceptado' ($O5.body.automata.alfabeto.Count -eq 2) "alfabeto=[$($O5.body.automata.alfabeto -join '')]"
Chk 'O5 valido con delta completa' ($O5.body.valido -eq $true) "errores=$($O5.body.errores -join '; ')"
$r = ProbarSimple $O5.body.id '|'
Chk 'O5 acepta la cadena pipe' ($r.body.recorridos[0].acepta -eq $true) "acepta=$($r.body.recorridos[0].acepta) traza=$($r.body.recorridos[0].traza -join '>')"

# pipe dentro del nombre de un estado
$O6 = Crear 'EstadoConPipe' @('a|b','c') @('x') 'a|b' @('c') @((Tr 'a|b' 'x' 'c'),(Tr 'c' 'x' 'c')) 1
Chk 'O6 pipe en nombre de estado' ($O6.body.automata.estados.Count -eq 2) "estados=$($O6.body.automata.estados -join ' , ')"
Chk 'O6 valido' ($O6.body.valido -eq $true) "errores=$($O6.body.errores -join '; ')"
Chk 'O6 nombre intacto' ($O6.body.automata.estados -contains 'a|b') "estados=$($O6.body.automata.estados -join ' , ')"

Chk 'O servidor vivo' (Vivo) 'murio en alfabetos grandes'

# =====================================================================
Seccion 'P. LENGUAJE VERIFICADO: BINARIO DIVISIBLE POR 3'
# =====================================================================
# r0,r1,r2 = resto mod 3. delta(ri,b) = r((2i+b) mod 3)
$trans = @()
for ($i = 0; $i -lt 3; $i++) {
  $trans += (Tr "r$i" '0' ("r" + ((2*$i) % 3)))
  $trans += (Tr "r$i" '1' ("r" + ((2*$i+1) % 3)))
}
$P1 = Crear 'Mod3' @('r0','r1','r2') @('0','1') 'r0' @('r0') $trans 1
Chk 'P1 mod3 valido' ($P1.body.valido -eq $true) "errores=$($P1.body.errores -join '; ')"
$idP1 = $P1.body.id

$malos = 0
$totalP = 0
for ($len = 1; $len -le 6; $len++) {
  $max = [math]::Pow(2, $len) - 1
  for ($n = 0; $n -le $max; $n++) {
    $s = [System.Convert]::ToString($n, 2).PadLeft($len, '0')
    $valor = [System.Convert]::ToInt64($s, 2)
    $esperado = (($valor % 3) -eq 0)
    $r = ProbarSimple $idP1 $s
    $totalP++
    $ok = ($r.body.recorridos[0].acepta -eq $esperado)
    if (-not $ok) { $malos++ }
    Chk ("P mod3 '$s' (=$valor)") $ok "acepta=$($r.body.recorridos[0].acepta) esperaba=$esperado traza=$($r.body.recorridos[0].traza -join '>')"
  }
}
Write-Host "  cadenas binarias verificadas: $totalP, discrepancias: $malos"

$r = ProbarSimple $idP1 ''
Chk 'P mod3 acepta epsilon (0 es divisible)' ($r.body.recorridos[0].acepta -eq $true) "acepta=$($r.body.recorridos[0].acepta)"

Chk 'P servidor vivo' (Vivo) 'murio en mod3'

# =====================================================================
Seccion 'Q. UNION = OR, VERIFICADO EXHAUSTIVAMENTE'
# =====================================================================
$Q1 = Crear 'TerminaA' @('t0','t1') @('a','b') 't0' @('t1') @((Tr 't0' 'a' 't1'),(Tr 't0' 'b' 't0'),(Tr 't1' 'a' 't1'),(Tr 't1' 'b' 't0')) 1
$Q2 = Crear 'ParesB' @('e0','e1') @('a','b') 'e0' @('e0') @((Tr 'e0' 'a' 'e0'),(Tr 'e0' 'b' 'e1'),(Tr 'e1' 'a' 'e1'),(Tr 'e1' 'b' 'e0')) 1
$idQ1 = $Q1.body.id; $idQ2 = $Q2.body.id
$QU = Post '/api/unir' "idA=$idQ1&idB=$idQ2"
Chk 'Q union creada' ($QU.body.exitoso -eq $true) "error=$($QU.body.error)"
$idQU = $QU.body.id

function Cadenas($maxLen) {
  $lista = @('')
  $actual = @('')
  for ($l = 1; $l -le $maxLen; $l++) {
    $nuevo = @()
    foreach ($p in $actual) { $nuevo += ($p + 'a'); $nuevo += ($p + 'b') }
    $lista += $nuevo
    $actual = $nuevo
  }
  return $lista
}

$todas = Cadenas 4
$discrepanciasOR = 0
$discrepanciasLeng = 0
foreach ($s in $todas) {
  $et = $s; if ($et -eq '') { $et = 'eps' }
  $r = Post '/api/probar-cadena' ("modo=union&idUnion=$idQU&idA=$idQ1&idB=$idQ2&cadena=" + (Enc $s))
  $ra = $r.body.recorridos[0].acepta
  $rb = $r.body.recorridos[1].acepta
  $ru = $r.body.recorridos[2].acepta

  # lenguajes esperados calculados aparte
  $espA = ($s.Length -gt 0) -and ($s[$s.Length-1] -eq 'a')
  $cuentaB = 0; foreach ($ch in $s.ToCharArray()) { if ($ch -eq 'b') { $cuentaB++ } }
  $espB = (($cuentaB % 2) -eq 0)

  if (($ra -ne $espA) -or ($rb -ne $espB)) { $discrepanciasLeng++ }
  Chk ("Q lenguajes '$et'") (($ra -eq $espA) -and ($rb -eq $espB)) "A=$ra(esp $espA) B=$rb(esp $espB)"
  if ($ru -ne ($ra -or $rb)) { $discrepanciasOR++ }
  Chk ("Q union=OR '$et'") ($ru -eq ($ra -or $rb)) "A=$ra B=$rb U=$ru"
}
Write-Host "  cadenas evaluadas: $($todas.Count)  fallos lenguaje: $discrepanciasLeng  fallos OR: $discrepanciasOR"

Chk 'Q servidor vivo' (Vivo) 'murio en la verificacion exhaustiva'

# =====================================================================
Seccion 'R. COLISION DE PARES: MAS VARIANTES'
# =====================================================================
# cada par de automatas deberia terminar bloqueado por validacion
$variantes = @(
  @{ n='coma simple';   a=@('x','x,y');       b=@('y,z','z');      s='m' },
  @{ n='coma doble';    a=@('p','p,q');       b=@('q,r','r');      s='m' },
  @{ n='parentesis';    a=@('(u','(u,v');     b=@('v)','v')        ; s='m' },
  @{ n='anidado';       a=@('(1,2)','(1');    b=@('2)','x')        ; s='m' }
)
foreach ($v in $variantes) {
  $ta = @(); foreach ($e in $v.a) { $ta += (Tr $e $v.s $v.a[0]) }
  $tb = @(); foreach ($e in $v.b) { $tb += (Tr $e $v.s $v.b[0]) }
  $ca = Crear ('ColA_' + $v.n) $v.a @($v.s) $v.a[0] @($v.a[0]) $ta 1
  $cb = Crear ('ColB_' + $v.n) $v.b @($v.s) $v.b[0] @($v.b[0]) $tb 1
  Chk ("R base valida A: " + $v.n) ($ca.body.valido -eq $true) "errores=$($ca.body.errores -join '; ')"
  Chk ("R base valida B: " + $v.n) ($cb.body.valido -eq $true) "errores=$($cb.body.errores -join '; ')"
  $u = Post '/api/unir' "idA=$($ca.body.id)&idB=$($cb.body.id)"
  Chk ("R union no revienta: " + $v.n) (-not $u.muerto) 'servidor cayo'
  if ($u.body.exitoso -eq $true) {
    # si se dejo pasar, el resultado tiene que ser valido de verdad
    $v2 = Post '/api/validar' "id=$($u.body.id)"
    Chk ("R si se guarda es valido: " + $v.n) ($v2.body.valido -eq $true) "se guardo un automata invalido: $($v2.body.errores -join '; ')"
  } else {
    Chk ("R bloqueo explicado: " + $v.n) ($u.body.error.Length -gt 0) 'sin mensaje de error'
  }
}

# nombres con parentesis que NO colisionan: la union debe funcionar normal
$ok1 = Crear 'ParenOk1' @('(a)','(b)') @('m') '(a)' @('(b)') @((Tr '(a)' 'm' '(b)'),(Tr '(b)' 'm' '(a)')) 1
$ok2 = Crear 'ParenOk2' @('[c]','[d]') @('m') '[c]' @('[d]') @((Tr '[c]' 'm' '[d]'),(Tr '[d]' 'm' '[c]')) 1
$uok = Post '/api/unir' "idA=$($ok1.body.id)&idB=$($ok2.body.id)"
Chk 'R parentesis sin colision se une bien' ($uok.body.exitoso -eq $true) "error=$($uok.body.error)"
Chk 'R resultado con 4 estados' ($uok.body.automata.estados.Count -eq 4) "estados=$($uok.body.automata.estados.Count)"

Chk 'R servidor vivo' (Vivo) 'murio en colisiones'

# =====================================================================
Seccion 'S. INVALIDOS: TODA COMBINACION'
# =====================================================================
$invalidos = @(
  @{ n='sin estados';              e=@();            s=@('a');     i=$null;  f=@();      t=@() },
  @{ n='sin inicial';              e=@('q0');        s=@('a');     i=$null;  f=@();      t=@((Tr 'q0' 'a' 'q0')) },
  @{ n='inicial inexistente';      e=@('q0');        s=@('a');     i='zz';   f=@();      t=@((Tr 'q0' 'a' 'q0')) },
  @{ n='delta vacia';              e=@('q0');        s=@('a');     i='q0';   f=@();      t=@() },
  @{ n='delta parcial';            e=@('q0','q1');   s=@('a','b'); i='q0';   f=@();      t=@((Tr 'q0' 'a' 'q1')) },
  @{ n='destino fantasma';         e=@('q0');        s=@('a');     i='q0';   f=@();      t=@((Tr 'q0' 'a' 'q7')) },
  @{ n='no determinista';          e=@('q0','q1');   s=@('a');     i='q0';   f=@();      t=@((Tr 'q0' 'a' 'q0'),(Tr 'q0' 'a' 'q1'),(Tr 'q1' 'a' 'q1')) },
  @{ n='triple destino';           e=@('q0','q1','q2'); s=@('a');  i='q0';   f=@();      t=@((Tr 'q0' 'a' 'q0'),(Tr 'q0' 'a' 'q1'),(Tr 'q0' 'a' 'q2'),(Tr 'q1' 'a' 'q1'),(Tr 'q2' 'a' 'q2')) },
  @{ n='inicial y destino malos';  e=@('q0');        s=@('a');     i='no';   f=@();      t=@((Tr 'q0' 'a' 'tampoco')) },
  @{ n='todo mal';                 e=@('q0','q1');   s=@('a','b'); i='x';    f=@('y');   t=@((Tr 'q0' 'a' 'z'),(Tr 'q0' 'a' 'q1')) }
)
foreach ($caso in $invalidos) {
  $r = Crear ('Inv_' + $caso.n) $caso.e $caso.s $caso.i $caso.f $caso.t 1
  Chk ("S invalido: " + $caso.n) ($r.body.valido -eq $false) "valido=$($r.body.valido) errores=$($r.body.errores -join '; ')"
  Chk ("S no guardado: " + $caso.n) ($r.body.guardado -eq $false) "guardado=$($r.body.guardado)"
  Chk ("S da explicacion: " + $caso.n) ($r.body.errores.Count -ge 1) "errores=$($r.body.errores.Count)"
}

Chk 'S servidor vivo' (Vivo) 'murio en invalidos'

# =====================================================================
Seccion 'T. DETERMINISMO E IDEMPOTENCIA'
# =====================================================================
$r1 = ProbarSimple $idP1 '110'
$r2 = ProbarSimple $idP1 '110'
Chk 'T misma consulta da misma traza' (($r1.body.recorridos[0].traza -join '>') -eq ($r2.body.recorridos[0].traza -join '>')) 'trazas distintas'
Chk 'T mismo veredicto' ($r1.body.recorridos[0].acepta -eq $r2.body.recorridos[0].acepta) 'veredictos distintos'

$u1 = Post '/api/unir' "idA=$idQ1&idB=$idQ2"
$u2 = Post '/api/unir' "idA=$idQ1&idB=$idQ2"
Chk 'T unir dos veces da el mismo automata' (($u1.body.automata.estados -join ',') -eq ($u2.body.automata.estados -join ',')) 'estados distintos'
Chk 'T unir dos veces crea ids distintos' ($u1.body.id -ne $u2.body.id) 'reuso el mismo id'

$v1 = Post '/api/validar' "id=$idP1"
$v2 = Post '/api/validar' "id=$idP1"
Chk 'T validar es estable' ($v1.body.valido -eq $v2.body.valido) 'resultado inestable'

# union conmutativa en aceptacion (no en nombres)
$uAB = Post '/api/unir' "idA=$idQ1&idB=$idQ2"
$uBA = Post '/api/unir' "idA=$idQ2&idB=$idQ1"
$difs = 0
foreach ($s in (Cadenas 3)) {
  $ra = (ProbarSimple $uAB.body.id $s).body.recorridos[0].acepta
  $rb = (ProbarSimple $uBA.body.id $s).body.recorridos[0].acepta
  if ($ra -ne $rb) { $difs++ }
}
Chk 'T A∪B y B∪A aceptan lo mismo' ($difs -eq 0) "difieren en $difs cadenas"

Chk 'T servidor vivo' (Vivo) 'murio en idempotencia'

# =====================================================================
Seccion 'U. CONSISTENCIA GLOBAL'
# =====================================================================
$r = Get_ '/api/automatas'
$lista = $r.body.automatas
$invalidosGuardados = @()
foreach ($a in $lista) {
  $v = Post '/api/validar' "id=$($a.id)"
  if ($v.body.valido -ne $true) { $invalidosGuardados += "$($a.id):$($a.nombre)" }
}
Chk 'U ningun guardado es invalido' ($invalidosGuardados.Count -eq 0) "invalidos=$($invalidosGuardados -join ', ')"

$idsOk = $true
for ($i = 0; $i -lt $lista.Count; $i++) { if ($lista[$i].id -ne $i) { $idsOk = $false } }
Chk 'U ids consecutivos' $idsOk 'ids desalineados'

# cada automata guardado: delta total verificada de forma independiente.
# ojo: -ceq / -ccontains, porque -eq en PowerShell ignora mayusculas y el
# modelo en c++ si las distingue.
$deltaRota = @()
foreach ($a in $lista) {
  $det = (Get_ ("/api/automata?id=" + $a.id)).body.automata
  foreach ($e in $det.estados) {
    foreach ($sim in $det.alfabeto) {
      $cuenta = 0
      foreach ($t in $det.transiciones) { if (($t.origen -ceq $e) -and ($t.simbolo -ceq $sim)) { $cuenta++ } }
      if ($cuenta -ne 1) { $deltaRota += "$($a.nombre)[$e,$sim]=$cuenta" }
    }
  }
}
Chk 'U delta total y determinista en todos' ($deltaRota.Count -eq 0) "rotas=$($deltaRota -join ' ')"

# cada inicial y cada final pertenece a Q
$fuera = @()
foreach ($a in $lista) {
  $det = (Get_ ("/api/automata?id=" + $a.id)).body.automata
  if (-not ($det.estados -ccontains $det.inicial)) { $fuera += "$($a.nombre):inicial" }
  foreach ($f in $det.finales) { if (-not ($det.estados -ccontains $f)) { $fuera += "$($a.nombre):final $f" } }
}
Chk 'U inicial y finales dentro de Q' ($fuera.Count -eq 0) "fuera=$($fuera -join ' ')"

# el modelo distingue mayusculas: se comprueba explicitamente
$casos = Crear 'CaseSensible' @('s','S') @('x','X') 's' @('S') @((Tr 's' 'x' 'S'),(Tr 's' 'X' 's'),(Tr 'S' 'x' 'S'),(Tr 'S' 'X' 's')) 1
Chk 'U estados s y S distintos' ($casos.body.automata.estados.Count -eq 2) "estados=$($casos.body.automata.estados -join ',')"
Chk 'U simbolos x y X distintos' ($casos.body.automata.alfabeto.Count -eq 2) "alfabeto=$($casos.body.automata.alfabeto -join ',')"
Chk 'U valido con 4 transiciones' ($casos.body.valido -eq $true) "errores=$($casos.body.errores -join '; ')"
$r = ProbarSimple $casos.body.id 'x'
Chk 'U minuscula x acepta' ($r.body.recorridos[0].acepta -eq $true) "acepta=$($r.body.recorridos[0].acepta)"
$r = ProbarSimple $casos.body.id 'X'
Chk 'U mayuscula X rechaza' ($r.body.recorridos[0].acepta -eq $false) "acepta=$($r.body.recorridos[0].acepta)"

Chk 'U servidor vivo al final' (Vivo) 'el servidor no sobrevivio'

Write-Host ""
Write-Host "======================================================"
Write-Host "  automatas en el servidor: $($lista.Count)"
Write-Host "  pruebas pasadas : $script:pasadas"
Write-Host "  pruebas fallidas: $script:fallidas"
Write-Host "======================================================"
if ($script:fallidas -gt 0) {
  Write-Host ""
  Write-Host "DETALLE DE FALLOS:"
  foreach ($f in $script:fallos) { Write-Host "  - $f" }
}
