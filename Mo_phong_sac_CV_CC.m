% Thông s? pack pin 2S
V_cell_max = 4.2;         % ?i?n áp t?i ?a c?a m?i cell (V)
V_pack_max = V_cell_max * 2; % ?i?n áp t?i ?a c?a pack 2S (V)
I_max = 1.04;             % Dòng s?c t?i ?a (A)
C_bat = 2;                % Dung l??ng pin (Ah)
SOC_initial = 0.2;        % Tr?ng thái s?c ban ??u (SOC)
T_initial = 25;           % Nhi?t ?? ban ??u (?? C)
T_max_increase = 15;      % T?ng nhi?t ?? t?i ?a trong quá trình s?c (gi? s? ??n gi?n)

% Gi?i h?n SOC khi ??t ?i?n áp t?i ?a (tr?ng thái tr??c giai ?o?n CV)
SOC_at_Vmax = 0.8;

% Th?i gian mô ph?ng
t_total = 2 * 3600;       % T?ng th?i gian mô ph?ng (giây)
dt = 1;                   % B??c th?i gian (giây)
time = 0:dt:t_total;

% Kh?i t?o bi?n
V_pack = zeros(size(time));
I_charge = zeros(size(time));
SOC = zeros(size(time));
Temperature = zeros(size(time));
SOC(1) = SOC_initial;
Temperature(1) = T_initial;

% S?c CC-CV cho pack 2S
for k = 2:length(time)
    % Tính ?i?n áp pack d?a vào SOC (??n gi?n hóa tuy?n tính)
    V_pack(k) = V_pack_max * min(SOC(k-1), SOC_at_Vmax);
    
    % Giai ?o?n s?c CC
    if SOC(k-1) < SOC_at_Vmax
        I_charge(k) = I_max;
    else
        % Giai ?o?n s?c CV: Dòng s?c gi?m d?n
        I_charge(k) = max(0, I_max * (1 - (SOC(k-1) - SOC_at_Vmax) / (1 - SOC_at_Vmax)));
    end
    
    % C?p nh?t SOC
    SOC(k) = SOC(k-1) + (I_charge(k) * dt) / (C_bat * 3600);
    SOC(k) = min(SOC(k), 1);  % Gi?i h?n SOC t?i ?a là 100%
    
    % C?p nh?t nhi?t ?? (gi? s? nhi?t ?? t?ng theo dòng s?c)
    if I_charge(k) > 0
        Temperature(k) = Temperature(k-1) + (T_max_increase / t_total) * dt;
    else
        % Nhi?t ?? gi?m d?n khi s?c xong
        Temperature(k) = max(T_initial, Temperature(k-1) - (T_max_increase / t_total) * dt);
    end
end

% V? ?? th?
figure;

% ?? th? Voltage
subplot(4,1,1);
plot(time/3600, V_pack, 'r', 'LineWidth', 1.5);  % ?? th? ?i?n áp màu ??
xlabel('Th?i gian (gi?)');
ylabel('?i?n áp (V)');
title('Bi?u ?? ?i?n áp (Voltage)');
grid on;
ylim([0 9]);                      
yticks(0:1:9);                    

% ?? th? Current
subplot(4,1,2);
plot(time/3600, I_charge, 'b', 'LineWidth', 1.5);  % ?? th? dòng s?c màu xanh
xlabel('Th?i gian (gi?)');
ylabel('Dòng s?c (A)');
title('Bi?u ?? Dòng ?i?n (Current)');
grid on;
ylim([0 1.2]);                    
yticks(0:0.2:1.2);                

% ?? th? SOC
subplot(4,1,3);
plot(time/3600, SOC * 100, 'g', 'LineWidth', 1.5);  % ?? th? SOC màu xanh lá
xlabel('Th?i gian (gi?)');
ylabel('M?c s?c SOC (%)');
title('Tr?ng thái s?c (SOC)');
grid on;
yticks(0:20:100);                 

% ?? th? Temperature
subplot(4,1,4);
plot(time/3600, Temperature, 'Color', [1 0.5 0], 'LineWidth', 1.5);  % ?? th? nhi?t ?? màu cam
xlabel('Th?i gian (gi?)');
ylabel('Nhi?t ?? (°C)');
title('Bi?u ?? Nhi?t ??');
grid on;
ylim([0 40]);                     
yticks(0:10:40);                  